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

#pragma once

#include <projectexplorer/kitmanager.h>

namespace Rust {
namespace Internal {

class ToolChainManager;

class TargetArchInformation final : public ProjectExplorer::KitInformation
{
public:
    static const char ID[];

    explicit TargetArchInformation(ToolChainManager& toolChainManager);

    QVariant defaultValue(const ProjectExplorer::Kit *kit) const override;
    QList<ProjectExplorer::Task> validate(const ProjectExplorer::Kit *kit) const override;
    ItemList toUserOutput(const ProjectExplorer::Kit *kit) const override;
    ProjectExplorer::KitConfigWidget *createConfigWidget(ProjectExplorer::Kit *kit) const override;

    static Core::Id getTargetArch(const ProjectExplorer::Kit* kit);
    static void setTargetArch(ProjectExplorer::Kit* kit, Core::Id id);

private:
    ToolChainManager& m_toolChainManager;
};

} // namespace Internal
} // namespace Rust
