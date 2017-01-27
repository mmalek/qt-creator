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
#include "buildconfigurationwidget.h"
#include "buildstep.h"

#include <projectexplorer/namedwidget.h>
#include <projectexplorer/project.h>
#include <projectexplorer/target.h>

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
    const QString buildType = map.value(BUILD_TYPE_KEY).toString();
    if (buildType == BUILD_TYPE_VAL_DEBUG) {
        setBuildType(Debug);
    } else if (buildType == BUILD_TYPE_VAL_RELEASE) {
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
        map.insert(BUILD_TYPE_KEY, BUILD_TYPE_VAL_DEBUG);
        break;
    case Release:
        map.insert(BUILD_TYPE_KEY, BUILD_TYPE_VAL_RELEASE);
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

}
