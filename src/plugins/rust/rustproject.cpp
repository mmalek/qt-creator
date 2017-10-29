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

//#include <projectexplorer/buildconfiguration.h>
//#include <projectexplorer/kit.h>
//#include <projectexplorer/projectexplorerconstants.h>
//#include <projectexplorer/projectnodes.h>
//#include <projectexplorer/projecttree.h>
//#include <projectexplorer/target.h>

#include <utils/algorithm.h>
//#include <utils/fileutils.h>
//#include <utils/hostosinfo.h>
#include <utils/runextensions.h>
#include <utils/qtcassert.h>

//#include <QDirIterator>
//#include <QFileInfo>
//#include <QTimer>

namespace Rust {
namespace Internal {
namespace {

const int MIN_TIME_BETWEEN_PROJECT_SCANS = 4500;

} // namespace

Project::Project(const Utils::FileName &fileName)
    : ProjectExplorer::Project(MimeTypes::RUST_SOURCE, fileName)
{
    setId("Rust.Project");

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

bool Project::supportsKit(ProjectExplorer::Kit *kit, QString *errorMessage) const
{
    Q_UNUSED(errorMessage)
    return kit->isValid();
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
        collectProjectFiles();
    }
}

void Project::collectProjectFiles()
{
    m_lastProjectScan.start();
    QTC_ASSERT(!m_futureWatcher.future().isRunning(), return);
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

    auto newRoot = new ProjectNode(projectDirectory());
    connect(newRoot, &ProjectNode::changed, this, &Project::scheduleProjectScan);
    newRoot->setDisplayName(displayName());
    newRoot->addNestedNodes(fileNodes);
    setRootProjectNode(newRoot);
    emit parsingFinished();
}

} // namespace Internal
} // namespace Rust
