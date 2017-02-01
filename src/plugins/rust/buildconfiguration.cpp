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

#include "buildconfiguration.h"
#include "buildstep.h"
#include "mimetypes.h"
#include "project.h"
#include "ui_buildconfigurationwidget.h"

#include <projectexplorer/buildinfo.h>
#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/kit.h>
#include <projectexplorer/project.h>
#include <projectexplorer/target.h>
#include <utils/qtcassert.h>
#include <utils/mimetypes/mimedatabase.h>

#include <QScopedPointer>

namespace Rust {

namespace {

const char BUILD_TYPE_KEY[] = "Rust.BuildConfiguration.BuildType";
const char BUILD_TYPE_VAL_DEBUG[] = "Debug";
const char BUILD_TYPE_VAL_RELEASE[] = "Release";

} // namespace

const char BuildConfiguration::ID[] = "Rust.BuildConfiguration";

BuildConfiguration::BuildConfiguration(ProjectExplorer::Target *target, BuildType buildType)
    : ProjectExplorer::BuildConfiguration(target, ID),
      m_buildType(buildType)
{
    updateBuildDirectory();
}

BuildConfiguration::BuildConfiguration(ProjectExplorer::Target *target, BuildConfiguration *source)
    : ProjectExplorer::BuildConfiguration(target, source),
      m_buildType(source->m_buildType)
{
    updateBuildDirectory();
}

ProjectExplorer::NamedWidget *BuildConfiguration::createConfigWidget()
{
    return new BuildConfigurationWidget(this);
}

bool BuildConfiguration::fromMap(const QVariantMap &map)
{
    const QString buildType = map.value(QLatin1String(BUILD_TYPE_KEY)).toString();
    if (buildType == QLatin1String(BUILD_TYPE_VAL_DEBUG)) {
        setBuildType(Debug);
    } else if (buildType == QLatin1String(BUILD_TYPE_VAL_RELEASE)) {
        setBuildType(Release);
    }

    return ProjectExplorer::BuildConfiguration::fromMap(map);
}

QVariantMap BuildConfiguration::toMap() const
{
    QVariantMap map(ProjectExplorer::BuildConfiguration::toMap());
    switch(m_buildType)
    {
    case Debug:
        map.insert(QLatin1String(BUILD_TYPE_KEY), QLatin1String(BUILD_TYPE_VAL_DEBUG));
        break;
    case Release:
        map.insert(QLatin1String(BUILD_TYPE_KEY), QLatin1String(BUILD_TYPE_VAL_RELEASE));
        break;
    default:
        break;
    }
    return map;
}

void BuildConfiguration::setBuildType(BuildType buildType)
{
    if (m_buildType != buildType) {
        m_buildType = buildType;
        updateBuildDirectory();
        emit buildTypeChanged();
    }
}

BuildConfiguration::BuildType BuildConfiguration::buildType() const
{
    return m_buildType;
}

Utils::FileName BuildConfiguration::buildDirectory(Utils::FileName path, BuildType buildType)
{
    path.appendPath(QLatin1String("target"));
    path.appendPath(BuildConfiguration::buildTypeName(buildType));
    return path;
}

void BuildConfiguration::updateBuildDirectory()
{
    setBuildDirectory(buildDirectory(target()->project()->projectDirectory(), m_buildType));
}

BuildConfigurationWidget::BuildConfigurationWidget(BuildConfiguration *buildConfiguration)
    : m_ui(new Ui::BuildConfigurationWidget)
{
    constexpr int BUILD_TYPE_DEBUG = 0;
    constexpr int BUILD_TYPE_RELEASE = 1;

    m_ui->setupUi(this);

    if (buildConfiguration->buildType() == BuildConfiguration::Debug) {
        m_ui->buildVariant->setCurrentIndex(BUILD_TYPE_DEBUG);
    } else if (buildConfiguration->buildType() == BuildConfiguration::Release) {
        m_ui->buildVariant->setCurrentIndex(BUILD_TYPE_RELEASE);
    }

    connect(m_ui->buildVariant,
            static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [buildConfiguration](int index) {
        if (index == BUILD_TYPE_DEBUG) {
            buildConfiguration->setBuildType(BuildConfiguration::Debug);
        } else if (index == BUILD_TYPE_RELEASE) {
            buildConfiguration->setBuildType(BuildConfiguration::Release);
        }
    });
}

BuildConfigurationWidget::~BuildConfigurationWidget()
{
}

BuildConfigurationFactory::BuildConfigurationFactory(QObject *parent)
    : ProjectExplorer::IBuildConfigurationFactory(parent)
{

}
int BuildConfigurationFactory::priority(const ProjectExplorer::Target *parent) const
{
    return canHandle(parent) ? 0 : -1;
}

QList<ProjectExplorer::BuildInfo *> BuildConfigurationFactory::availableBuilds(const ProjectExplorer::Target *parent) const
{
    return { createBuildInfo(parent->kit(), parent->project()->projectDirectory(), BuildConfiguration::Debug) };
}

int BuildConfigurationFactory::priority(const ProjectExplorer::Kit *k, const QString &projectPath) const
{
    Utils::MimeDatabase mdb;
    if (k && mdb.mimeTypeForFile(projectPath).matchesName(QLatin1String(MimeTypes::CARGO_MANIFEST)))
        return 0;
    else
        return -1;
}

QList<ProjectExplorer::BuildInfo *> BuildConfigurationFactory::availableSetups(const ProjectExplorer::Kit *k, const QString &projectPath) const
{
    const Utils::FileName prjDir = Utils::FileName::fromString(projectPath).parentDir();

    ProjectExplorer::BuildInfo *debug = createBuildInfo(k, prjDir, BuildConfiguration::Debug);
    debug->displayName = tr("Debug");

    ProjectExplorer::BuildInfo *release = createBuildInfo(k, prjDir, BuildConfiguration::Release);
    release->displayName = tr("Release");

    return { debug, release };
}

ProjectExplorer::BuildConfiguration *BuildConfigurationFactory::create(ProjectExplorer::Target *parent, const ProjectExplorer::BuildInfo *info) const
{
    QTC_ASSERT(info->factory() == this, return 0);
    QTC_ASSERT(info->kitId == parent->kit()->id(), return 0);
    QTC_ASSERT(!info->displayName.isEmpty(), return 0);

    BuildConfiguration* buildConfiguration = new BuildConfiguration(parent, info->buildType);
    buildConfiguration->setDisplayName(info->displayName);
    buildConfiguration->setDefaultDisplayName(info->displayName);

    Utils::FileName buildDir = info->buildDirectory;
    if (buildDir.isEmpty()) {
        buildDir = buildDirectory(parent->project()->projectDirectory(), info->buildType);
    }
    buildConfiguration->setBuildDirectory(buildDir);

    ProjectExplorer::BuildStepList* buildSteps = buildConfiguration->stepList(ProjectExplorer::Constants::BUILDSTEPS_BUILD);
    buildSteps->appendStep(new BuildStep(buildSteps));

    ProjectExplorer::BuildStepList* cleanSteps = buildConfiguration->stepList(ProjectExplorer::Constants::BUILDSTEPS_CLEAN);
    cleanSteps->appendStep(new CleanStep(cleanSteps));

    return buildConfiguration;
}

bool BuildConfigurationFactory::canRestore(const ProjectExplorer::Target *parent, const QVariantMap &map) const
{
    return canHandle(parent) && ProjectExplorer::idFromMap(map) == BuildConfiguration::ID;
}

ProjectExplorer::BuildConfiguration *BuildConfigurationFactory::restore(ProjectExplorer::Target *parent, const QVariantMap &map)
{
    QTC_ASSERT(canRestore(parent, map), return nullptr);
    QScopedPointer<BuildConfiguration> result(new BuildConfiguration(parent));
    return result->fromMap(map) ? result.take() : nullptr;
}

bool BuildConfigurationFactory::canClone(const ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *source) const
{
    return canHandle(parent) && qobject_cast<BuildConfiguration *>(source);
}

ProjectExplorer::BuildConfiguration *BuildConfigurationFactory::clone(ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *source)
{
    if (!canClone(parent, source))
        return 0;
    BuildConfiguration *buildConfiguration = static_cast<BuildConfiguration *>(source);
    return new BuildConfiguration(parent, buildConfiguration);
}

bool BuildConfigurationFactory::canHandle(const ProjectExplorer::Target *t) const
{
    return qobject_cast<Project *>(t->project());
}

Utils::FileName BuildConfigurationFactory::buildDirectory(const Utils::FileName &projectDir,
                                                          BuildConfiguration::BuildType buildType)
{
    Utils::FileName path = projectDir;
    path.appendPath(QLatin1String("target"));
    path.appendPath(BuildConfiguration::buildTypeName(buildType));
    return path;
}

ProjectExplorer::BuildInfo *BuildConfigurationFactory::createBuildInfo(const ProjectExplorer::Kit *k,
                                                                       const Utils::FileName &projectDir,
                                                                       ProjectExplorer::BuildConfiguration::BuildType buildType) const
{
    ProjectExplorer::BuildInfo* result = new ProjectExplorer::BuildInfo(this);
    result->buildType = buildType;
    result->kitId = k->id();
    result->typeName = tr("Build");
    result->buildDirectory = buildDirectory(projectDir, buildType);
    return result;
}

}
