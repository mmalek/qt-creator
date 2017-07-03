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

#include "rustkitinformation.h"
#include "rustkitconfigwidget.h"
#include "rusttoolchainmanager.h"

#include <projectexplorer/kit.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/task.h>

namespace Rust {
namespace Internal {

const char KitInformation::ID[] = "Rust.KitInformation";

KitInformation::KitInformation(ToolChainManager &toolChainManager)
    : m_toolChainManager(toolChainManager)
{
    setObjectName(QLatin1String("RustKitInformation"));
    setId(ID);
    setPriority(17000);
}

QVariant KitInformation::defaultValue(const ProjectExplorer::Kit *kit) const
{
    const ToolChain* defaultToolChain = m_toolChainManager.getDefault();
    if (kit && defaultToolChain) {
        return defaultToolChain->id.toSetting();
    } else {
        return Core::Id().toSetting();
    }
}

QList<ProjectExplorer::Task> KitInformation::validate(const ProjectExplorer::Kit *kit) const
{
    QList<ProjectExplorer::Task> result;
    const ToolChain *tool = m_toolChainManager.get(getToolChain(kit));
    if (!tool) {
        ProjectExplorer::Task task(ProjectExplorer::Task::Warning,
                                   tr("No Rust toolchain set up"),
                                   Utils::FileName(),
                                   -1,
                                   Core::Id(ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM));
        result.push_back(task);
    }
    return result;
}

KitInformation::ItemList KitInformation::toUserOutput(const ProjectExplorer::Kit *kit) const
{
    const ToolChain* toolChain = m_toolChainManager.get(getToolChain(kit));
    QString name;
    if (!toolChain) {
        name = tr("None");
    } else if (toolChain->fullToolChainName.isEmpty()) {
        name = toolChain->name;
    } else {
        name = QString("%1 %2").arg(toolChain->name).arg(toolChain->fullToolChainName);
    }
    return {Item(tr("Rust"), std::move(name))};
}

ProjectExplorer::KitConfigWidget *KitInformation::createConfigWidget(ProjectExplorer::Kit *kit) const
{
    return new KitConfigWidget(m_toolChainManager, kit, this);
}

Core::Id KitInformation::getToolChain(const ProjectExplorer::Kit *kit)
{
    if (kit) {
        return Core::Id::fromSetting(kit->value(ID));
    } else {
        return Core::Id();
    }

}

void KitInformation::setToolChain(ProjectExplorer::Kit *kit, Core::Id id)
{
    if (kit) {
        kit->setValue(ID, id.toSetting());
    }
}

} // namespace Internal
} // namespace Rust
