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

namespace Rust {
namespace Internal {

ProjectNode::ProjectNode(const Utils::FileName &projectFilePath)
    : ProjectExplorer::ProjectNode(projectFilePath)
{}

QList<ProjectExplorer::ProjectAction> ProjectNode::supportedActions(Node *node) const
{
    using namespace ProjectExplorer;

    switch (node->nodeType()) {
    case FileNodeType:
        return { Rename, RemoveFile };
    case FolderNodeType:
    case ProjectNodeType:
        return { AddNewFile, RemoveFile };
    default:
        return ProjectNode::supportedActions(node);
    }
}

bool ProjectNode::addFiles(const QStringList &filePaths, QStringList *notAdded)
{
    Q_UNUSED(filePaths)
    Q_UNUSED(notAdded)
    return true;
}

bool ProjectNode::removeFiles(const QStringList &filePaths, QStringList *notRemoved)
{
    Q_UNUSED(filePaths)
    Q_UNUSED(notRemoved)
    return true;
}

bool ProjectNode::deleteFiles(const QStringList &filePaths)
{
    Q_UNUSED(filePaths)
    return true;
}

bool ProjectNode::renameFile(const QString &filePath, const QString &newFilePath)
{
    Q_UNUSED(filePath)
    Q_UNUSED(newFilePath)
    return true;
}

} // namespace Internal
} // namespace Rust
