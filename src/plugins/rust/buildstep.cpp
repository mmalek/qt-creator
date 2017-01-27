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

#include "buildstep.h"
#include "rustcparser.hpp"

#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/processparameters.h>
#include <projectexplorer/project.h>

namespace Rust {

const char BuildStep::ID[] = "Rust.BuildStep";
const char BuildStep::DISPLAY_NAME[] = QT_TRANSLATE_NOOP("RustBuildStep", "cargo build");

BuildStep::BuildStep(ProjectExplorer::BuildStepList *parentList)
    : AbstractProcessStep(parentList, ID)
{
    setDefaultDisplayName(tr(DISPLAY_NAME));
    setDisplayName(tr(DISPLAY_NAME));
}

bool BuildStep::init(QList<const ProjectExplorer::BuildStep *> &earlierSteps)
{
    QStringList args;
    args.append(QLatin1String("build"));
    args.append(QLatin1String("--message-format=json"));
    if (buildConfiguration()->buildType() == ProjectExplorer::BuildConfiguration::Release) {
        args.append(QLatin1String("--release"));
    }

    processParameters()->setCommand(QLatin1String("cargo"));
    processParameters()->setArguments(args.join(' '));
    processParameters()->setWorkingDirectory(project()->projectDirectory().toString());
    processParameters()->setEnvironment(buildConfiguration()->environment());

    setOutputParser(new RustcParser);

    return AbstractProcessStep::init(earlierSteps);
}

ProjectExplorer::BuildStepConfigWidget *BuildStep::createConfigWidget()
{
    return new ProjectExplorer::SimpleBuildStepConfigWidget(this);
}

const char CleanStep::ID[] = "Rust.CleanStep";
const char CleanStep::DISPLAY_NAME[] = QT_TRANSLATE_NOOP("RustCleanStep", "cargo clean");

CleanStep::CleanStep(ProjectExplorer::BuildStepList *parentList)
    : AbstractProcessStep(parentList, ID)
{
    setDefaultDisplayName(tr(DISPLAY_NAME));
    setDisplayName(tr(DISPLAY_NAME));
}

bool CleanStep::init(QList<const ProjectExplorer::BuildStep *> &earlierSteps)
{
    QStringList args;
    args.append(QLatin1String("clean"));
    if (buildConfiguration()->buildType() == ProjectExplorer::BuildConfiguration::Release) {
        args.append(QLatin1String("--release"));
    }

    processParameters()->setCommand(QLatin1String("cargo"));
    processParameters()->setArguments(args.join(' '));
    processParameters()->setWorkingDirectory(project()->projectDirectory().toString());
    processParameters()->setEnvironment(buildConfiguration()->environment());

    return AbstractProcessStep::init(earlierSteps);
}

ProjectExplorer::BuildStepConfigWidget *CleanStep::createConfigWidget()
{
    return new ProjectExplorer::SimpleBuildStepConfigWidget(this);
}

} // namespace Rust
