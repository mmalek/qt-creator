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

namespace  Rust {

class CargoStep : public ProjectExplorer::AbstractProcessStep
{
    Q_OBJECT

public:
    CargoStep(ProjectExplorer::BuildStepList *bsl, Core::Id id, const QString& displayName);
    CargoStep(ProjectExplorer::BuildStepList *bsl, CargoStep *bs, const QString& displayName);

    bool init(QList<const ProjectExplorer::BuildStep *> &earlierSteps) final;

    ProjectExplorer::BuildStepConfigWidget *createConfigWidget() override;

    bool fromMap(const QVariantMap &map) override;
    QVariantMap toMap() const override;

    const QString& extraArgs() const { return m_extraArgs; }
    virtual QString mainArgs() const = 0;

public slots:
    void setExtraArgs(const QString& value);

private:
    QString m_extraArgs;
};

class BuildStep final : public CargoStep
{
    Q_OBJECT

public:
    static const char ID[];
    static const char DISPLAY_NAME[];

    explicit BuildStep(ProjectExplorer::BuildStepList *bsl);
    BuildStep(ProjectExplorer::BuildStepList *bsl, BuildStep *bs);

    QString mainArgs() const override;
};

class TestStep final : public CargoStep
{
    Q_OBJECT

public:
    static const char ID[];
    static const char DISPLAY_NAME[];

    explicit TestStep(ProjectExplorer::BuildStepList *bsl);
    TestStep(ProjectExplorer::BuildStepList *bsl, TestStep *bs);

    QString mainArgs() const override;
};

class BenchStep final : public CargoStep
{
    Q_OBJECT

public:
    static const char ID[];
    static const char DISPLAY_NAME[];

    explicit BenchStep(ProjectExplorer::BuildStepList *bsl);
    BenchStep(ProjectExplorer::BuildStepList *bsl, BenchStep *bs);

    QString mainArgs() const override;
};

class CleanStep final : public CargoStep
{
    Q_OBJECT

public:
    static const char ID[];
    static const char DISPLAY_NAME[];

    explicit CleanStep(ProjectExplorer::BuildStepList *bsl);
    CleanStep(ProjectExplorer::BuildStepList *bsl, CleanStep *bs);

    QString mainArgs() const override;
};

namespace Ui { class BuildStepConfigWidget; }

class BuildStepConfigWidget : public ProjectExplorer::SimpleBuildStepConfigWidget
{
    Q_OBJECT

public:
    explicit BuildStepConfigWidget(CargoStep *step);
    ~BuildStepConfigWidget();

    bool showWidget() const override { return true; }

private:
    QScopedPointer<Ui::BuildStepConfigWidget> m_ui;
};

class BuildStepFactory final : public ProjectExplorer::IBuildStepFactory
{
public:
    explicit BuildStepFactory(QObject *parent = nullptr);

    QList<ProjectExplorer::BuildStepInfo> availableSteps(ProjectExplorer::BuildStepList *parent) const override;
    ProjectExplorer::BuildStep *create(ProjectExplorer::BuildStepList *parent, Core::Id id) override;
    ProjectExplorer::BuildStep *clone(ProjectExplorer::BuildStepList *parent, ProjectExplorer::BuildStep *source) override;
};

}
