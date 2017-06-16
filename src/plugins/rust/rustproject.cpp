/****************************************************************************
**
** Copyright (C) Filippo Cucchetto <filippocucchetto@gmail.com>
** Copyright (C) Michal Malek <michalm@fastmail.fm>
** Contact: http://www.qt.io/licensing
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "rustproject.h"
#include "rustkitinformation.h"
#include "rustprojectnode.h"
#include "rustprojectmanager.h"
#include "rusttoolchainmanager.h"

#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/kit.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/target.h>
#include <texteditor/textdocument.h>

#include <utils/algorithm.h>
#include <utils/environment.h>

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QQueue>

using namespace ProjectExplorer;
using namespace Utils;

namespace Rust {
namespace Internal {

const int MIN_TIME_BETWEEN_PROJECT_SCANS = 4500;

Project::Project(ProjectManager *projectManager, const QString &fileName)
    : m_cargoReadManifest(new QtcProcess(this))
{
    setId("Rust.Project");
    setProjectManager(projectManager);
    setDocument(new TextEditor::TextDocument);
    document()->setFilePath(FileName::fromString(fileName));

    QFileInfo fi = QFileInfo(fileName);
    QDir dir = fi.dir();
    setRootProjectNode(new ProjectNode(FileName::fromString(dir.absolutePath())));
    rootProjectNode()->setDisplayName(dir.dirName());

    m_projectScanTimer.setSingleShot(true);
    connect(&m_projectScanTimer, &QTimer::timeout, this, &Project::populateProject);
    connect(m_cargoReadManifest, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(readManifestFinished(int, QProcess::ExitStatus)));

    m_cargoReadManifest->setCommand(QLatin1String("cargo"), QLatin1String("read-manifest"));
    m_cargoReadManifest->setEnvironment(Utils::Environment::systemEnvironment());
    m_cargoReadManifest->setWorkingDirectory(projectDirectory().toString());

    populateProject();

    connect(&m_fsWatcher, &QFileSystemWatcher::directoryChanged, this, &Project::scheduleProjectScan);
}

QString Project::displayName() const
{
    return rootProjectNode()->displayName();
}

QStringList Project::files(FilesMode) const
{
    return QStringList(m_files.toList());
}

bool Project::needsConfiguration() const
{
    return targets().empty();
}

void Project::readManifestFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);

    if (exitStatus != QProcess::NormalExit) {
        return;
    }

    QByteArray output = m_cargoReadManifest->readAllStandardOutput();

    const QJsonDocument document = QJsonDocument::fromJson(output);
    if (document.isNull() || !document.isObject()) {
        return;
    }

    const QJsonObject root = document.object();
    const QString name = root.value(QLatin1String("name")).toString();
    if (ProjectExplorer::ProjectNode *node = rootProjectNode()) {
        node->setDisplayName(name);
    }

    m_products.clear();

    for (QJsonValue targetVal : root.value(QLatin1String("targets")).toArray()) {
        const QJsonObject target = targetVal.toObject();
        const QJsonArray kindArr = target.value(QLatin1String("kind")).toArray();
        if (!kindArr.empty()) {
            const QString name = target.value(QLatin1String("name")).toString();
            const QString srcPathStr = target.value(QLatin1String("src_path")).toString();
            const Utils::FileName srcPath = Utils::FileName::fromString(srcPathStr);
            const QString kindStr = kindArr.first().toString();
            if (kindStr == QLatin1String("bench")) {
                m_products.append(Product{Product::Benchmark, name, srcPath});
            } else if (kindStr == QLatin1String("bin")) {
                m_products.append(Product{Product::Binary, name, srcPath});
            } else if (kindStr == QLatin1String("example")) {
                m_products.append(Product{Product::Example, name, srcPath});
            } else if (kindStr == QLatin1String("lib")) {
                m_products.append(Product{Product::Library, name, srcPath});
            } else  if (kindStr == QLatin1String("test")) {
                m_products.append(Product{Product::Test, name, srcPath});
            }
        }
    }
}

void Project::scheduleProjectScan()
{
    auto elapsedTime = m_lastProjectScan.elapsed();
    if (elapsedTime < MIN_TIME_BETWEEN_PROJECT_SCANS) {
        if (!m_projectScanTimer.isActive()) {
            m_projectScanTimer.setInterval(MIN_TIME_BETWEEN_PROJECT_SCANS - elapsedTime);
            m_projectScanTimer.start();
        }
    } else {
        populateProject();
    }
}

void Project::populateProject()
{
    m_lastProjectScan.start();

    QSet<QString> oldFiles = m_files;
    m_files.clear();
    recursiveScanDirectory(QDir(projectDirectory().toString()), m_files);

    if (m_files == oldFiles)
        return;

    QList<FileNode *> fileNodes = Utils::transform(m_files.toList(), [](const QString &f) {
        return new FileNode(FileName::fromString(f), SourceType, false);
    });
    rootProjectNode()->buildTree(fileNodes);

    m_cargoReadManifest->start();

    emit fileListChanged();

    emit parsingFinished();
}

void Project::recursiveScanDirectory(const QDir &dir, QSet<QString> &container)
{
    for (const QFileInfo &info : dir.entryInfoList(QDir::AllDirs |
                                                   QDir::Files |
                                                   QDir::NoDotAndDotDot |
                                                   QDir::NoSymLinks |
                                                   QDir::CaseSensitive)) {
        if (info.isDir())
            recursiveScanDirectory(QDir(info.filePath()), container);
        else
            container << info.filePath();
    }
    m_fsWatcher.addPath(dir.absolutePath());
}

bool Project::supportsKit(Kit *kit, QString *errorMessage) const
{
    const ProjectManager& pm = static_cast<const ProjectManager&>(*projectManager());
    if (pm.toolChainManager().get(KitInformation::getToolChain(kit))) {
        return kit->isValid();
    } else {
        if (errorMessage) {
            *errorMessage = tr("No Rust toolchain set.");
        }
        return false;
    }
}

FileNameList Project::files() const
{
    FileNameList result;

    QQueue<FolderNode *> folders;
    folders.enqueue(rootProjectNode());

    while (!folders.isEmpty()) {
        FolderNode *folder = folders.takeFirst();
        for (FileNode *file : folder->fileNodes()) {
            if (file->displayName().endsWith(QLatin1String(".rs")))
                result.append(file->filePath());
        }
        folders.append(folder->subFolderNodes());
    }

    return result;
}

} // namespace Internal
} // namespace Rust
