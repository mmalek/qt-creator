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

#include <projectexplorer/abstractprocessstep.h>

namespace  Rust {

class BuildStep final : public ProjectExplorer::AbstractProcessStep
{
    Q_OBJECT

public:
    static const char ID[];
    static const char DISPLAY_NAME[];

    BuildStep(ProjectExplorer::BuildStepList *parentList);

    bool init(QList<const ProjectExplorer::BuildStep *> &earlierSteps) override;

    ProjectExplorer::BuildStepConfigWidget *createConfigWidget() override;
};

class CleanStep final : public ProjectExplorer::AbstractProcessStep
{
    Q_OBJECT

public:
    static const char ID[];
    static const char DISPLAY_NAME[];

    CleanStep(ProjectExplorer::BuildStepList *parentList);

    bool init(QList<const ProjectExplorer::BuildStep *> &earlierSteps) override;

    ProjectExplorer::BuildStepConfigWidget *createConfigWidget() override;
};

class BuildStepFactory final : public ProjectExplorer::IBuildStepFactory
{
public:
    explicit BuildStepFactory(QObject *parent = nullptr);

    QList<ProjectExplorer::BuildStepInfo> availableSteps(ProjectExplorer::BuildStepList *parent) const override;
    ProjectExplorer::BuildStep *create(ProjectExplorer::BuildStepList *parent, Core::Id id) override;
    ProjectExplorer::BuildStep *clone(ProjectExplorer::BuildStepList *parent, ProjectExplorer::BuildStep *product) override;
};

}
