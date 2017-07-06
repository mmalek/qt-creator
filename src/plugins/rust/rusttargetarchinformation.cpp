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

#include "rusttargetarchinformation.h"
#include "rusttargetarchwidget.h"
#include "rusttoolchainmanager.h"

#include <projectexplorer/kit.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/task.h>

namespace Rust {
namespace Internal {

const char TargetArchInformation::ID[] = "Rust.TargetArchInformation";

TargetArchInformation::TargetArchInformation(ToolChainManager &toolChainManager)
    : m_toolChainManager(toolChainManager)
{
    setObjectName(QLatin1String("RustTargetArchInformation"));
    setId(ID);
    setPriority(16000);
}

QVariant TargetArchInformation::defaultValue(const ProjectExplorer::Kit *kit) const
{
    Q_UNUSED(kit)
    return Core::Id().toSetting();
}

QList<ProjectExplorer::Task> TargetArchInformation::validate(const ProjectExplorer::Kit *kit) const
{
    Q_UNUSED(kit)
    return {};
}

TargetArchInformation::ItemList TargetArchInformation::toUserOutput(const ProjectExplorer::Kit *kit) const
{
    const TargetArch* targetArch = m_toolChainManager.targetArch(getTargetArch(kit));
    QString name;
    if (!targetArch) {
        name = tr("Default");
    } else {
        name = targetArch->name;
    }
    return {Item(tr("Rust target architecture"), std::move(name))};
}

ProjectExplorer::KitConfigWidget *TargetArchInformation::createConfigWidget(ProjectExplorer::Kit *kit) const
{
    return new TargetArchWidget(m_toolChainManager, kit, this);
}

Core::Id TargetArchInformation::getTargetArch(const ProjectExplorer::Kit *kit)
{
    if (kit) {
        return Core::Id::fromSetting(kit->value(ID));
    } else {
        return Core::Id();
    }

}

void TargetArchInformation::setTargetArch(ProjectExplorer::Kit *kit, Core::Id id)
{
    if (kit) {
        kit->setValue(ID, id.toSetting());
    }
}

} // namespace Internal
} // namespace Rust
