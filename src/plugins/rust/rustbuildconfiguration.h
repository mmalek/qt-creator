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

#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/namedwidget.h>

#include <QScopedPointer>

namespace Rust {
namespace Internal {

class ToolChainManager;

class BuildConfiguration final : public ProjectExplorer::BuildConfiguration
{
    Q_OBJECT

public:
    static const char ID[];

    explicit BuildConfiguration(ProjectExplorer::Target *target, BuildType buildType = Unknown);
    BuildConfiguration(ProjectExplorer::Target *target, BuildConfiguration *source);

    ProjectExplorer::NamedWidget *createConfigWidget() override;

    bool fromMap(const QVariantMap &map) override;
    QVariantMap toMap() const override;

    void setBuildType(BuildType buildType);
    BuildType buildType() const override;

    static Utils::FileName buildDirectory(Utils::FileName projectDir, BuildType buildType);

private:
    void updateBuildDirectory();

    BuildType m_buildType = Unknown;
};

namespace Ui { class BuildConfigurationWidget; }

class BuildConfigurationWidget final : public ProjectExplorer::NamedWidget
{
    Q_OBJECT

public:
    explicit BuildConfigurationWidget(BuildConfiguration *buildConfiguration);
    ~BuildConfigurationWidget();

private:
    QScopedPointer<Ui::BuildConfigurationWidget> m_ui;
};

class BuildConfigurationFactory final : public ProjectExplorer::IBuildConfigurationFactory
{
    Q_OBJECT

public:
    explicit BuildConfigurationFactory(const ToolChainManager& tcm, QObject *parent = nullptr);

    int priority(const ProjectExplorer::Target *parent) const override;
    QList<ProjectExplorer::BuildInfo *> availableBuilds(const ProjectExplorer::Target *parent) const override;
    int priority(const ProjectExplorer::Kit *k, const QString &projectPath) const override;
    QList<ProjectExplorer::BuildInfo *> availableSetups(const ProjectExplorer::Kit *k, const QString &projectPath) const override;
    ProjectExplorer::BuildConfiguration *create(ProjectExplorer::Target *parent, const ProjectExplorer::BuildInfo *info) const override;
    bool canRestore(const ProjectExplorer::Target *parent, const QVariantMap &map) const override;
    ProjectExplorer::BuildConfiguration *restore(ProjectExplorer::Target *parent, const QVariantMap &map) override;
    bool canClone(const ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *source) const override;
    ProjectExplorer::BuildConfiguration *clone(ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *source) override;

private:
    bool canHandle(const ProjectExplorer::Target *t) const;
    static Utils::FileName buildDirectory(const Utils::FileName &projectDir,
                                          ProjectExplorer::BuildConfiguration::BuildType buildType);
    ProjectExplorer::BuildInfo *createBuildInfo(const ProjectExplorer::Kit *k,
                                                const Utils::FileName &projectDir,
                                                ProjectExplorer::BuildConfiguration::BuildType buildType) const;

private:
    const ToolChainManager& m_toolChainManager;
};

} // namespace Internal
} // namespace Rust
