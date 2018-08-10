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

#include <projectexplorer/runconfiguration.h>

class QLabel;

namespace Utils { class FileName; }

namespace Rust {
namespace Internal {

struct Product;
class Project;

class RunConfiguration final : public ProjectExplorer::RunConfiguration
{
    Q_OBJECT

public:
    RunConfiguration(ProjectExplorer::Target *parent);

    bool isConfigured() const override;
    QWidget *createConfigurationWidget() override;
    ProjectExplorer::Runnable runnable() const override;
    bool fromMap(const QVariantMap &map) override;
    Utils::FileName executable(ProjectExplorer::BuildConfiguration* bc) const;
    Utils::FileName workingDirectory() const;
    const Project& rustProject() const;
    const Product* product() const { return m_product; }

private:
    QString defaultDisplayName() const;

    const Product *m_product = nullptr;
};

class RunConfigurationFactory final : public ProjectExplorer::IRunConfigurationFactory
{
    Q_OBJECT

public:
    RunConfigurationFactory(QObject *parent = nullptr);

    QList<ProjectExplorer::RunConfigurationCreationInfo> availableCreators(ProjectExplorer::Target *parent) const override;

    bool canHandle(ProjectExplorer::Target *target) const override;
};

class RunConfigurationWidget : public QWidget
{
    Q_OBJECT

public:
    RunConfigurationWidget(RunConfiguration *rc);

private slots:
    void onActiveBuildConfigChanged(ProjectExplorer::BuildConfiguration *bc);

private:
    void runConfigurationEnabledChange();

    RunConfiguration *m_rc;
    QLabel *m_executableLineLabel;
    bool m_isShown = false;
};

} // namespace Internal
} // namespace Rust
