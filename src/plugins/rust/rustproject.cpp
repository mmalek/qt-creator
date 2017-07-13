/****************************************************************************
**
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
#include "rustprojectnode.h"
#include "rustprojectmanager.h"
#include "rusttoolchainmanager.h"

#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/kit.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectnodes.h>
#include <projectexplorer/target.h>
#include <texteditor/textdocument.h>

#include <utils/algorithm.h>
#include <utils/fileutils.h>

#include <QDirIterator>
#include <QFileInfo>
#include <QTimer>

namespace Rust {
namespace Internal {

Project::Project(ProjectManager *projectManager, Manifest manifest)
    : m_products(std::move(manifest.products))
{
    setId("Rust.Project");
    setProjectManager(projectManager);
    setDocument(new TextEditor::TextDocument);
    document()->setFilePath(manifest.file);

    setRootProjectNode(new ProjectNode(manifest.directory));
    rootProjectNode()->setDisplayName(manifest.name);

    buildProjectTree(projectDirectory().toString());

    QTimer::singleShot(0, this, SIGNAL(parsingFinished()));

    connect(&m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged,
            this, &Project::buildProjectTree);
}

QString Project::displayName() const
{
    return rootProjectNode()->displayName();
}

QStringList Project::files(FilesMode fileMode) const
{
    const auto isExpected = [fileMode](const ProjectExplorer::FileNode* fileNode) {
        return ((fileMode | SourceFiles) && fileNode->fileType() == ProjectExplorer::SourceType) ||
               ((fileMode | GeneratedFiles) && fileNode->isGenerated());
    };

    const auto toFilePath = [](const ProjectExplorer::FileNode* fileNode) {
        return fileNode->filePath().toString();
    };

    return Utils::transform(Utils::filtered(rootProjectNode()->recursiveFileNodes(), isExpected),
                            toFilePath);
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

void Project::recursiveScanDirectory(const QString &path,
                                     QList<ProjectExplorer::FileNode*> &fileNodes,
                                     bool topDir,
                                     bool inTargetDir)
{
    QDirIterator dirIt(path, QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    while (dirIt.hasNext()) {
        dirIt.next();

        const QFileInfo info = dirIt.fileInfo();

        if (info.isDir()) {
            recursiveScanDirectory(info.filePath(), fileNodes, false,
                                   inTargetDir || (topDir && info.fileName() == QLatin1String("target")));
        } else if (info.suffix() != QLatin1String("autosave") &&
             info.fileName().compare(QLatin1String("Cargo.toml.user"), Qt::CaseInsensitive) != 0) {

            Utils::FileName filePath(info);

            ProjectExplorer::FileType fileType;
            if (info.suffix() == QLatin1String("rs"))
                fileType = ProjectExplorer::SourceType;
            else if (info.fileName().compare(QLatin1String("Cargo.toml"), Qt::CaseInsensitive) == 0)
                fileType = ProjectExplorer::ProjectFileType;
            else
                fileType = ProjectExplorer::UnknownFileType;

            bool generated = inTargetDir ||
                    info.fileName().compare(QLatin1String("Cargo.lock"), Qt::CaseInsensitive) == 0;

            fileNodes.append(new ProjectExplorer::FileNode(filePath, fileType, generated));
        }
    }

    m_fileSystemWatcher.addPath(path);
}

void Project::buildProjectTree(const QString &path)
{
    QFileInfo info(path);
    if (info.exists()) {
        if (ProjectExplorer::FolderNode* node = rootProjectNode()->findOrCreateSubFolderNode(path)) {
            QList<ProjectExplorer::FileNode*> fileNodes;
            recursiveScanDirectory(path, fileNodes, projectDirectory().toString() == path);
            node->buildTree(fileNodes);
        }
    }

    emit fileListChanged();
}

} // namespace Internal
} // namespace Rust
