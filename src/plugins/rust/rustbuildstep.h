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

#include <QScopedPointer>

namespace Rust {
namespace Internal {

class ToolChainManager;

class CargoStep : public ProjectExplorer::AbstractProcessStep
{
    Q_OBJECT

public:
    CargoStep(ProjectExplorer::BuildStepList *bsl, Core::Id id, const QString& displayName);

    bool init(QList<const ProjectExplorer::BuildStep *> &earlierSteps) final;

    ProjectExplorer::BuildStepConfigWidget *createConfigWidget() override;

    bool fromMap(const QVariantMap &map) override;
    QVariantMap toMap() const override;

    QString extraArgs() const;
    bool showJsonOutput() const { return m_showJsonOnConsole; }
    virtual QStringList mainArgs() const = 0;

    const ToolChainManager &toolChainManager() const;

public slots:
    void setExtraArgs(const QString& value);
    void setShowJsonOutput(bool value);

protected:
    void stdOutput(const QString &line) override;

private:
    QStringList m_extraArgs;
    bool m_showJsonOnConsole;
};

class BuildStep final : public CargoStep
{
    Q_OBJECT

public:
    static const char ID[];
    static const char DISPLAY_NAME[];

    explicit BuildStep(ProjectExplorer::BuildStepList *bsl);

    QStringList mainArgs() const override;
};

class BuildStepFactory final : public ProjectExplorer::BuildStepFactory
{
public:
    BuildStepFactory();
};

class TestStep final : public CargoStep
{
    Q_OBJECT

public:
    static const char ID[];
    static const char DISPLAY_NAME[];

    explicit TestStep(ProjectExplorer::BuildStepList *bsl);

    QStringList mainArgs() const override;
};

class TestStepFactory final : public ProjectExplorer::BuildStepFactory
{
public:
    TestStepFactory();
};

class BenchStep final : public CargoStep
{
    Q_OBJECT

public:
    static const char ID[];
    static const char DISPLAY_NAME[];

    explicit BenchStep(ProjectExplorer::BuildStepList *bsl);

    QStringList mainArgs() const override;
};

class BenchStepFactory final : public ProjectExplorer::BuildStepFactory
{
public:
    BenchStepFactory();
};

class CleanStep final : public CargoStep
{
    Q_OBJECT

public:
    static const char ID[];
    static const char DISPLAY_NAME[];

    explicit CleanStep(ProjectExplorer::BuildStepList *bsl);

    QStringList mainArgs() const override;
};

class CleanStepFactory final : public ProjectExplorer::BuildStepFactory
{
public:
    CleanStepFactory();
};

namespace Ui { class BuildStepConfigWidget; }

class BuildStepConfigWidget : public ProjectExplorer::SimpleBuildStepConfigWidget
{
    Q_OBJECT

public:
    explicit BuildStepConfigWidget(CargoStep *step);
    ~BuildStepConfigWidget() override;

    bool showWidget() const override { return true; }

private:
    QScopedPointer<Ui::BuildStepConfigWidget> m_ui;
};

} // namespace Internal
} // namespace Rust
