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

#include "rustbuildstep.h"
#include "rustcompileroutputparser.h"
#include "ui_rustbuildstepconfigwidget.h"

#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/processparameters.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <utils/qtcassert.h>

namespace Rust {
namespace Internal {

namespace {

Q_CONSTEXPR QLatin1String EXTRA_ARGS_KEY("Rust.CargoStep.ExtraArgs");
Q_CONSTEXPR QLatin1String SHOW_JSON_OUTPUT_KEY("Rust.CargoStep.ShowJsonOutput");

} // namespace

CargoStep::CargoStep(ProjectExplorer::BuildStepList *bsl, Core::Id id, const QString &displayName)
    : AbstractProcessStep(bsl, id),
      m_showJsonOnConsole(false)
{
    setDefaultDisplayName(displayName);
    setDisplayName(displayName);
}

CargoStep::CargoStep(ProjectExplorer::BuildStepList *bsl, CargoStep *bs, const QString &displayName)
    : AbstractProcessStep(bsl, bs),
      m_extraArgs(bs->m_extraArgs),
      m_showJsonOnConsole(bs->m_showJsonOnConsole)
{
    setDefaultDisplayName(displayName);
    setDisplayName(displayName);
}

bool CargoStep::init(QList<const ProjectExplorer::BuildStep *> &earlierSteps)
{
    processParameters()->setCommand(QLatin1String("cargo"));
    processParameters()->setWorkingDirectory(project()->projectDirectory().toString());
    processParameters()->setEnvironment(buildConfiguration()->environment());

    if (!extraArgs().isEmpty()) {
        QString arguments = QString("%1 %2").arg(mainArgs().trimmed()).arg(extraArgs().trimmed());
        processParameters()->setArguments(arguments);
    } else {
        processParameters()->setArguments(mainArgs().trimmed());
    }

    setOutputParser(new CompilerOutputParser);

    return AbstractProcessStep::init(earlierSteps);
}

ProjectExplorer::BuildStepConfigWidget *CargoStep::createConfigWidget()
{
    return new BuildStepConfigWidget(this);
}

bool CargoStep::fromMap(const QVariantMap &map)
{
    setExtraArgs(map.value(EXTRA_ARGS_KEY).toString());
    setShowJsonOutput(map.value(SHOW_JSON_OUTPUT_KEY).toBool());
    return ProjectExplorer::AbstractProcessStep::fromMap(map);
}

QVariantMap CargoStep::toMap() const
{
    QVariantMap map = ProjectExplorer::AbstractProcessStep::toMap();
    map.insert(EXTRA_ARGS_KEY, extraArgs());
    map.insert(SHOW_JSON_OUTPUT_KEY, showJsonOutput());
    return map;
}

void CargoStep::setExtraArgs(const QString &value)
{
    m_extraArgs = value;
}

void CargoStep::setShowJsonOutput(bool value)
{
    m_showJsonOnConsole = value;
}

void CargoStep::stdOutput(const QString &line)
{
    if (CompilerOutputParser::isParsable(line)) {
        if (ProjectExplorer::IOutputParser* parser = outputParser()) {
            parser->stdOutput(line);
        }

        if (m_showJsonOnConsole) {
            emit addOutput(line, BuildStep::NormalOutput, BuildStep::DontAppendNewline);
        }
    } else {
        ProjectExplorer::AbstractProcessStep::stdOutput(line);
    }
}

const char BuildStep::ID[] = "Rust.BuildStep";
const char BuildStep::DISPLAY_NAME[] = "cargo build";

BuildStep::BuildStep(ProjectExplorer::BuildStepList *bsl)
    : CargoStep(bsl, ID, QLatin1String(DISPLAY_NAME))
{
}

BuildStep::BuildStep(ProjectExplorer::BuildStepList *bsl, BuildStep *bs)
    : CargoStep(bsl, bs, QLatin1String(DISPLAY_NAME))
{
}

QString BuildStep::mainArgs() const
{
    QString args = QLatin1String("build --message-format=json");
    if (buildConfiguration()->buildType() == ProjectExplorer::BuildConfiguration::Release) {
        args.append(QLatin1String(" --release"));
    }
    return args;
}

const char TestStep::ID[] = "Rust.TestStep";
const char TestStep::DISPLAY_NAME[] = "cargo test";

TestStep::TestStep(ProjectExplorer::BuildStepList *bsl)
    : CargoStep(bsl, ID, QLatin1String(DISPLAY_NAME))
{
}

TestStep::TestStep(ProjectExplorer::BuildStepList *bsl, TestStep *bs)
    : CargoStep(bsl, bs, QLatin1String(DISPLAY_NAME))
{
}

QString TestStep::mainArgs() const
{
    QString args = QLatin1String("test --message-format=json");
    if (buildConfiguration()->buildType() == ProjectExplorer::BuildConfiguration::Release) {
        args.append(QLatin1String(" --release"));
    }
    return args;
}

const char BenchStep::ID[] = "Rust.BenchStep";
const char BenchStep::DISPLAY_NAME[] = "cargo bench";

BenchStep::BenchStep(ProjectExplorer::BuildStepList *bsl)
    : CargoStep(bsl, ID, QLatin1String(DISPLAY_NAME))
{
}

BenchStep::BenchStep(ProjectExplorer::BuildStepList *bsl, BenchStep *bs)
    : CargoStep(bsl, bs, QLatin1String(DISPLAY_NAME))
{
}

QString BenchStep::mainArgs() const
{
    return QLatin1String("bench --message-format=json");
}

const char CleanStep::ID[] = "Rust.CleanStep";
const char CleanStep::DISPLAY_NAME[] = "cargo clean";

CleanStep::CleanStep(ProjectExplorer::BuildStepList *bsl)
    : CargoStep(bsl, ID, QLatin1String(DISPLAY_NAME))
{
}

CleanStep::CleanStep(ProjectExplorer::BuildStepList *bsl, CleanStep *bs)
    : CargoStep(bsl, bs, QLatin1String(DISPLAY_NAME))
{
}

QString CleanStep::mainArgs() const
{
    QString args = QLatin1String("clean");
    if (buildConfiguration()->buildType() == ProjectExplorer::BuildConfiguration::Release) {
        args.append(QLatin1String(" --release"));
    }
    return args;
}

BuildStepConfigWidget::BuildStepConfigWidget(CargoStep *step) :
    ProjectExplorer::SimpleBuildStepConfigWidget(step),
    m_ui(new Ui::BuildStepConfigWidget)
{
    setContentsMargins(0, 0, 0, 0);

    m_ui->setupUi(this);

    m_ui->extraArgs->setText(step->extraArgs());
    m_ui->showJsonOutput->setChecked(step->showJsonOutput());

    connect(m_ui->extraArgs, &QLineEdit::textChanged, step, &CargoStep::setExtraArgs);
    connect(m_ui->showJsonOutput, &QCheckBox::clicked, step, &CargoStep::setShowJsonOutput);
}

BuildStepConfigWidget::~BuildStepConfigWidget()
{
}

BuildStepFactory::BuildStepFactory(QObject *parent)
    : ProjectExplorer::IBuildStepFactory(parent)
{
}

QList<ProjectExplorer::BuildStepInfo> BuildStepFactory::availableSteps(ProjectExplorer::BuildStepList *parent) const
{
    if (parent->id() == ProjectExplorer::Constants::BUILDSTEPS_BUILD) {
        return { { BuildStep::ID, QLatin1String(BuildStep::DISPLAY_NAME) },
                 { TestStep::ID, QLatin1String(TestStep::DISPLAY_NAME) },
                 { BenchStep::ID, QLatin1String(BenchStep::DISPLAY_NAME) } };
    } else if (parent->id() == ProjectExplorer::Constants::BUILDSTEPS_CLEAN) {
        return {{ CleanStep::ID, QLatin1String(CleanStep::DISPLAY_NAME) }};
    } else {
        return {};
    }
}

ProjectExplorer::BuildStep *BuildStepFactory::create(ProjectExplorer::BuildStepList *parent, Core::Id id)
{
    if (id == BuildStep::ID) {
        return new BuildStep(parent);
    } else if (id == TestStep::ID) {
        return new TestStep(parent);
    } else if (id == BenchStep::ID) {
        return new BenchStep(parent);
    } else if (id == CleanStep::ID) {
        return new CleanStep(parent);
    } else {
        return nullptr;
    }
}

ProjectExplorer::BuildStep *BuildStepFactory::clone(ProjectExplorer::BuildStepList *parent, ProjectExplorer::BuildStep *source)
{
    QTC_ASSERT(parent, return nullptr);
    QTC_ASSERT(source, return nullptr);

    QScopedPointer<ProjectExplorer::BuildStep> result;
    if (BuildStep* bs = qobject_cast<BuildStep*>(source)) {
        result.reset(new BuildStep(parent, bs));
    } else if (TestStep* bs = qobject_cast<TestStep*>(source)) {
        result.reset(new TestStep(parent, bs));
    } else if (BenchStep* bs = qobject_cast<BenchStep*>(source)) {
        result.reset(new BenchStep(parent, bs));
    } else if (CleanStep* bs = qobject_cast<CleanStep*>(source)) {
        result.reset(new CleanStep(parent, bs));
    }

    return result && result->fromMap(source->toMap()) ? result.take() : nullptr;
}

} // namespace Internal
} // namespace Rust
