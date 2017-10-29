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

#include <projectexplorer/projectnodes.h>
#include <projectexplorer/projectnodes.h>

namespace Rust {
namespace Internal {

ProjectNode::ProjectNode(const Utils::FileName &projectFilePath)
    : ProjectExplorer::ProjectNode(projectFilePath)
{}

bool ProjectNode::addFiles(const QStringList &filePaths, QStringList *notAdded)
{
    Q_UNUSED(filePaths)
    Q_UNUSED(notAdded)
    emit changed();
    return true;
}

bool ProjectNode::removeFiles(const QStringList &filePaths, QStringList *notRemoved)
{
    Q_UNUSED(filePaths)
    Q_UNUSED(notRemoved)
    emit changed();
    return true;
}

bool ProjectNode::deleteFiles(const QStringList &filePaths)
{
    Q_UNUSED(filePaths)
    emit changed();
    return true;
}

bool ProjectNode::renameFile(const QString &filePath, const QString &newFilePath)
{
    Q_UNUSED(filePath)
    Q_UNUSED(newFilePath)
    emit changed();
    return true;
}

bool ProjectNode::supportsAction(ProjectExplorer::ProjectAction action, ProjectExplorer::Node *node) const
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
