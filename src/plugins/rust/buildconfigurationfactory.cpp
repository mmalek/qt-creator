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

#include "buildconfigurationfactory.h"
#include "buildconfiguration.h"
#include "buildstep.h"

#include <projectexplorer/buildinfo.h>
#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/kit.h>
#include <projectexplorer/project.h>
#include <projectexplorer/target.h>
#include <utils/qtcassert.h>

#include <QScopedPointer>

namespace Rust {

BuildConfigurationFactory::BuildConfigurationFactory(QObject *parent)
    : ProjectExplorer::IBuildConfigurationFactory(parent)
{

}
int BuildConfigurationFactory::priority(const ProjectExplorer::Target *parent) const
{
    Q_UNUSED(parent);
    return 0;
}

QList<ProjectExplorer::BuildInfo *> BuildConfigurationFactory::availableBuilds(const ProjectExplorer::Target *parent) const
{
    return { createBuildInfo(parent->kit(), parent->project()->projectDirectory(), BuildConfiguration::Debug) };
}

int BuildConfigurationFactory::priority(const ProjectExplorer::Kit *k, const QString &projectPath) const
{
    Q_UNUSED(k);
    Q_UNUSED(projectPath);
    return 0;
}

QList<ProjectExplorer::BuildInfo *> BuildConfigurationFactory::availableSetups(const ProjectExplorer::Kit *k, const QString &projectPath) const
{
    ProjectExplorer::BuildInfo *debug = createBuildInfo(k, Utils::FileName::fromString(projectPath), BuildConfiguration::Debug);
    ProjectExplorer::BuildInfo *release = createBuildInfo(k, Utils::FileName::fromString(projectPath), BuildConfiguration::Release);
    return { debug, release };
}

ProjectExplorer::BuildConfiguration *BuildConfigurationFactory::create(ProjectExplorer::Target *parent, const ProjectExplorer::BuildInfo *info) const
{
    BuildConfiguration* buildConfiguration = new BuildConfiguration(parent);
    buildConfiguration->setDisplayName(info->displayName);
    buildConfiguration->setDefaultDisplayName(info->displayName);
    buildConfiguration->setBuildDirectory(info->buildDirectory);

    ProjectExplorer::BuildStepList* buildSteps = buildConfiguration->stepList(ProjectExplorer::Constants::BUILDSTEPS_BUILD);
    buildSteps->appendStep(new BuildStep(buildSteps));

    ProjectExplorer::BuildStepList* cleanSteps = buildConfiguration->stepList(ProjectExplorer::Constants::BUILDSTEPS_CLEAN);
    cleanSteps->appendStep(new CleanStep(cleanSteps));

    return buildConfiguration;
}

bool BuildConfigurationFactory::canRestore(const ProjectExplorer::Target *parent, const QVariantMap &map) const
{
    Q_UNUSED(parent);
    return ProjectExplorer::idFromMap(map) == BuildConfiguration::ID;
}

ProjectExplorer::BuildConfiguration *BuildConfigurationFactory::restore(ProjectExplorer::Target *parent, const QVariantMap &map)
{
    QTC_ASSERT(parent, return nullptr);
    QTC_ASSERT(canRestore(parent, map), return nullptr);
    QScopedPointer<BuildConfiguration> result(new BuildConfiguration(parent));
    return result->fromMap(map) ? result.take() : nullptr;
}

bool BuildConfigurationFactory::canClone(const ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *product) const
{
    QTC_ASSERT(parent, return false);
    QTC_ASSERT(product, return false);
    return product->id() == BuildConfiguration::ID;
}

ProjectExplorer::BuildConfiguration *BuildConfigurationFactory::clone(ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *product)
{
    QTC_ASSERT(parent, return nullptr);
    QTC_ASSERT(product, return nullptr);
    BuildConfiguration* buildConfiguration = qobject_cast<BuildConfiguration *>(product);
    QTC_ASSERT(buildConfiguration, return nullptr);
    QScopedPointer<BuildConfiguration> result(new BuildConfiguration(parent));
    return result->fromMap(buildConfiguration->toMap()) ? result.take() : nullptr;
}

ProjectExplorer::BuildInfo *BuildConfigurationFactory::createBuildInfo(const ProjectExplorer::Kit *k,
                                                                       const Utils::FileName &projectPath,
                                                                       ProjectExplorer::BuildConfiguration::BuildType buildType) const
{
    ProjectExplorer::BuildInfo* result = new ProjectExplorer::BuildInfo(this);
    result->buildType = buildType;
    result->displayName = BuildConfiguration::buildTypeName(buildType);
    result->buildDirectory = projectPath;
    result->kitId = k->id();
    result->typeName = tr("Build");
    return result;
}

} // namespace Rust
