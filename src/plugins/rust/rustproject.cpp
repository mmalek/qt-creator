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
#include "rustmanifest.h"
#include "rustmimetypes.h"
#include "rustprojectnode.h"
#include "rusttoolchainmanager.h"

#include <coreplugin/messagemanager.h>
#include <coreplugin/progressmanager/progressmanager.h>
#include <coreplugin/vcsmanager.h>

#include <utils/algorithm.h>
#include <utils/runextensions.h>
#include <utils/qtcassert.h>

namespace Rust {
namespace Internal {
namespace {

Q_CONSTEXPR std::chrono::milliseconds MIN_TIME_BETWEEN_PROJECT_SCANS{4500};

} // namespace

const char Project::ID[] = "Rust.Project";

Project::Project(const Utils::FileName &fileName)
    : ProjectExplorer::Project(MimeTypes::RUST_SOURCE, fileName)
{
    setId(ID);

    QString errorString;

    if (Manifest manifest = Manifest::read(fileName, &errorString)) {

        m_products = std::move(manifest.products);

        setDisplayName(manifest.name);

        m_projectScanTimer.setSingleShot(true);
        connect(&m_projectScanTimer, &QTimer::timeout, this, &Project::collectProjectFiles);

        connect(&m_futureWatcher, &QFutureWatcher<QList<ProjectExplorer::FileNode *>>::finished,
                this, &Project::updateProject);

        collectProjectFiles();
    } else {
        Core::MessageManager::write(tr("Cannot open Cargo manifest \"%1\": %2.")
                                    .arg(fileName.toString())
                                    .arg(errorString));
    }
}

bool Project::needsConfiguration() const
{
    return targets().empty();
}

bool Project::supportsKit(const ProjectExplorer::Kit *kit, QString *errorMessage) const
{
    Q_UNUSED(errorMessage)
    return kit->isValid();
}

bool Project::addFiles(const QStringList &filePaths, QStringList *notAdded)
{
    Q_UNUSED(filePaths)
    Q_UNUSED(notAdded)
    scheduleProjectScan();
    return true;
}

bool Project::removeFiles(const QStringList &filePaths, QStringList *notRemoved)
{
    Q_UNUSED(filePaths)
    Q_UNUSED(notRemoved)
    scheduleProjectScan();
    return true;
}

bool Project::deleteFiles(const QStringList &filePaths)
{
    Q_UNUSED(filePaths)
    scheduleProjectScan();
    return true;
}

bool Project::renameFile(const QString &filePath, const QString &newFilePath)
{
    Q_UNUSED(filePath)
    Q_UNUSED(newFilePath)
    scheduleProjectScan();
    return true;
}

void Project::scheduleProjectScan()
{
    auto elapsedTime = std::chrono::milliseconds{m_lastProjectScan.elapsed()};
    if (elapsedTime < MIN_TIME_BETWEEN_PROJECT_SCANS) {
        if (!m_projectScanTimer.isActive()) {
            m_projectScanTimer.start(MIN_TIME_BETWEEN_PROJECT_SCANS - elapsedTime);
        }
    } else {
        collectProjectFiles();
    }
}

void Project::collectProjectFiles()
{
    QTC_ASSERT(!m_futureWatcher.future().isRunning(), return);
    emitParsingStarted();
    m_lastProjectScan.start();
    Utils::FileName prjDir = projectDirectory();
    const QList<Core::IVersionControl *> versionControls = Core::VcsManager::versionControls();
    QFuture<QList<ProjectExplorer::FileNode *>> future = Utils::runAsync([prjDir, versionControls] {
        return ProjectExplorer::FileNode::scanForFilesWithVersionControls(
                    prjDir,
                    [](const Utils::FileName &fn) {
                        return new ProjectExplorer::FileNode(fn, ProjectExplorer::FileType::Source, false);
                    },
                    versionControls);
    });
    m_futureWatcher.setFuture(future);
    Core::ProgressManager::addTask(future, tr("Scanning for Rust files"), "Rust.Project.Scan");
}

void Project::updateProject()
{
    const QStringList oldFiles = m_files;
    m_files.clear();

    auto fileNodes = Utils::filtered(m_futureWatcher.future().result(),
                                     [&](const ProjectExplorer::FileNode *fn) {
        const Utils::FileName path = fn->filePath();
        const QString fileName = path.fileName();
        const bool keep = fileName.compare(QLatin1String("Cargo.toml.user"),
                                           Utils::HostOsInfo::fileNameCaseSensitivity()) != 0;
        if (!keep)
            delete fn;
        return keep;
    });

    m_files = Utils::transform(fileNodes,
                               [](const ProjectExplorer::FileNode *fn) { return fn->filePath().toString(); });
    Utils::sort(m_files, [](const QString &a, const QString &b) { return a < b; });

    if (oldFiles == m_files)
        return;

    auto newRoot = new ProjectNode(*this, projectDirectory());
    newRoot->setDisplayName(displayName());
    newRoot->addNestedNodes(fileNodes);
    setRootProjectNode(newRoot);
    emitParsingFinished(true);
}

} // namespace Internal
} // namespace Rust
