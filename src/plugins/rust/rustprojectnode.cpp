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

#include "rustprojectnode.h"
#include "rustproject.h"

#include <projectexplorer/projectnodes.h>
#include <projectexplorer/projectnodes.h>

namespace Rust {
namespace Internal {

ProjectNode::ProjectNode(Project &project, const Utils::FileName &projectFilePath)
    : ProjectExplorer::ProjectNode(projectFilePath)
    , m_project(project)
{}

bool ProjectNode::addFiles(const QStringList &filePaths, QStringList *notAdded)
{
    return m_project.addFiles(filePaths, notAdded);
}

bool ProjectNode::removeFiles(const QStringList &filePaths, QStringList *notRemoved)
{
    return m_project.removeFiles(filePaths, notRemoved);
}

bool ProjectNode::deleteFiles(const QStringList &filePaths)
{
    return m_project.deleteFiles(filePaths);
}

bool ProjectNode::renameFile(const QString &filePath, const QString &newFilePath)
{
    return m_project.renameFile(filePath, newFilePath);
}

bool ProjectNode::supportsAction(ProjectExplorer::ProjectAction action, const Node *node) const
{
    switch (node->nodeType()) {
    case ProjectExplorer::NodeType::File:
        return action == ProjectExplorer::Rename || action == ProjectExplorer::RemoveFile;
    case ProjectExplorer::NodeType::Folder:
    case ProjectExplorer::NodeType::Project:
        return action == ProjectExplorer::AddNewFile || action == ProjectExplorer::RemoveFile;
    default:
        return false;
    }
}

} // namespace Internal
} // namespace Rust
