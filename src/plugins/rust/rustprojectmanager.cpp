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

#include "rustprojectmanager.h"
#include "rustmanifest.h"
#include "rustmimetypes.h"
#include "rustproject.h"
#include "rusttoolchainmanager.h"

namespace Rust {
namespace Internal {

ProjectManager::ProjectManager(const ToolChainManager &toolChainManager)
    : m_toolChainManager(toolChainManager)
{
}

QString ProjectManager::mimeType() const
{
    return QLatin1String(MimeTypes::CARGO_MANIFEST);
}

ProjectExplorer::Project *ProjectManager::openProject(const QString &fileName, QString *errorString)
{
    QString subErrorString;

    if (Manifest manifest = Manifest::read(fileName, m_toolChainManager.environment(), &subErrorString)) {
        return new Project(this, std::move(manifest));
    }

    if (errorString) {
        *errorString = tr("Cannot open Cargo manifest \"%1\": %2.").arg(fileName)
                                                                   .arg(subErrorString);
    }

    return nullptr;
}

} // namespace Internal
} // namespace Rust
