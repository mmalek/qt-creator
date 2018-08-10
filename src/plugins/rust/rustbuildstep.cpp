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
#include "rustbuildconfiguration.h"
#include "rustcompileroutputparser.h"
#include "rustkitinformation.h"
#include "rustproject.h"
#include "rustsettings.h"
#include "rusttargetarchinformation.h"
#include "rusttoolchainmanager.h"
#include "ui_rustbuildstepconfigwidget.h"

#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/processparameters.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/target.h>
#include <utils/qtcassert.h>

namespace Rust {
namespace Internal {

namespace {

QLatin1String EXTRA_ARGS_KEY("Rust.CargoStep.ExtraArgs");
QLatin1String SHOW_JSON_OUTPUT_KEY("Rust.CargoStep.ShowJsonOutput");
Q_CONSTEXPR QLatin1Char ARGS_SEPARATOR(' ');

} // namespace

CargoStep::CargoStep(ProjectExplorer::BuildStepList *bsl, Core::Id id, const QString &displayName)
    : AbstractProcessStep(bsl, id),
      m_showJsonOnConsole(false)
{
    setDefaultDisplayName(displayName);
    setDisplayName(displayName);
}

bool CargoStep::init(QList<const ProjectExplorer::BuildStep *> &earlierSteps)
{
    ProjectExplorer::Kit* kit = target()->kit();

    processParameters()->setCommand(Settings::value(Settings::CARGO));

    QStringList arguments;

    const Core::Id toolChainId = KitInformation::getToolChain(kit);
    if (const ToolChain* toolChain = toolChainManager().toolChain(toolChainId)) {
        if (!toolChain->fullToolChainName.isEmpty()) {
            arguments.append(QString("+%1").arg(toolChain->fullToolChainName));
        }
    }

    arguments += mainArgs();

    const Core::Id targetArchId = TargetArchInformation::getTargetArch(kit);
    if (const TargetArch* targetArch = toolChainManager().targetArch(targetArchId)) {
        arguments.append(QString("--target=%1").arg(targetArch->name));
    }

    arguments += m_extraArgs;

    processParameters()->setWorkingDirectory(project()->projectDirectory().toString());
    processParameters()->setEnvironment(buildConfiguration()->environment());
    processParameters()->setArguments(arguments.join(ARGS_SEPARATOR));

    setOutputParser(new CompilerOutputParser);

    return AbstractProcessStep::init(earlierSteps);
}

ProjectExplorer::BuildStepConfigWidget *CargoStep::createConfigWidget()
{
    return new BuildStepConfigWidget(this);
}

bool CargoStep::fromMap(const QVariantMap &map)
{
    m_extraArgs = map.value(EXTRA_ARGS_KEY).toStringList();
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

QString CargoStep::extraArgs() const
{
    return m_extraArgs.join(ARGS_SEPARATOR);
}

const ToolChainManager &CargoStep::toolChainManager() const
{
    ToolChainManager *tcm = ToolChainManager::instance();
    Q_ASSERT(tcm);
    return *tcm;
}

void CargoStep::setExtraArgs(const QString &value)
{
    m_extraArgs = value.split(ARGS_SEPARATOR, QString::SkipEmptyParts);
}

void CargoStep::setShowJsonOutput(bool value)
{
    m_showJsonOnConsole = value;
}

void CargoStep::stdOutput(const QString &line)
{
    const bool parsable = CompilerOutputParser::isParsable(line);

    if (parsable) {
        if (ProjectExplorer::IOutputParser* parser = outputParser()) {
            parser->stdOutput(line);
        }
    }

    if (m_showJsonOnConsole || !parsable) {
        emit addOutput(line, BuildStep::OutputFormat::Stdout, BuildStep::DontAppendNewline);
    }
}

const char BuildStep::ID[] = "Rust.BuildStep";
const char BuildStep::DISPLAY_NAME[] = "cargo build";

BuildStep::BuildStep(ProjectExplorer::BuildStepList *bsl)
    : CargoStep(bsl, ID, QLatin1String(DISPLAY_NAME))
{
}

QStringList BuildStep::mainArgs() const
{
    QStringList args { QLatin1String("build"), QLatin1String("--message-format=json") };
    if (buildConfiguration()->buildType() == ProjectExplorer::BuildConfiguration::Release) {
        args.append(QLatin1String("--release"));
    }
    return args;
}

BuildStepFactory::BuildStepFactory()
{
    registerStep<BuildStep>(BuildStep::ID);
    setDisplayName(BuildStep::DISPLAY_NAME);
    setSupportedStepList(ProjectExplorer::Constants::BUILDSTEPS_BUILD);
    setSupportedConfiguration(BuildConfiguration::ID);
    setSupportedProjectType(Project::ID);
}

const char TestStep::ID[] = "Rust.TestStep";
const char TestStep::DISPLAY_NAME[] = "cargo test";

TestStep::TestStep(ProjectExplorer::BuildStepList *bsl)
    : CargoStep(bsl, ID, QLatin1String(DISPLAY_NAME))
{
}

QStringList TestStep::mainArgs() const
{
    QStringList args { QLatin1String("test"), QLatin1String("--message-format=json") };
    if (buildConfiguration()->buildType() == ProjectExplorer::BuildConfiguration::Release) {
        args.append(QLatin1String("--release"));
    }
    return args;
}

TestStepFactory::TestStepFactory()
{
    registerStep<TestStep>(TestStep::ID);
    setDisplayName(TestStep::DISPLAY_NAME);
    setSupportedStepList(ProjectExplorer::Constants::BUILDSTEPS_BUILD);
    setSupportedConfiguration(BuildConfiguration::ID);
    setSupportedProjectType(Project::ID);
}

const char BenchStep::ID[] = "Rust.BenchStep";
const char BenchStep::DISPLAY_NAME[] = "cargo bench";

BenchStep::BenchStep(ProjectExplorer::BuildStepList *bsl)
    : CargoStep(bsl, ID, QLatin1String(DISPLAY_NAME))
{
}

QStringList BenchStep::mainArgs() const
{
    return { QLatin1String("bench"), QLatin1String("--message-format=json") };
}

BenchStepFactory::BenchStepFactory()
{
    registerStep<BenchStep>(BenchStep::ID);
    setDisplayName(BenchStep::DISPLAY_NAME);
    setSupportedStepList(ProjectExplorer::Constants::BUILDSTEPS_BUILD);
    setSupportedConfiguration(BuildConfiguration::ID);
    setSupportedProjectType(Project::ID);
}

const char CleanStep::ID[] = "Rust.CleanStep";
const char CleanStep::DISPLAY_NAME[] = "cargo clean";

CleanStep::CleanStep(ProjectExplorer::BuildStepList *bsl)
    : CargoStep(bsl, ID, QLatin1String(DISPLAY_NAME))
{
}

QStringList CleanStep::mainArgs() const
{
    QStringList args { QLatin1String("clean") };
    if (buildConfiguration()->buildType() == ProjectExplorer::BuildConfiguration::Release) {
        args.append(QLatin1String("--release"));
    }
    return args;
}

CleanStepFactory::CleanStepFactory()
{
    registerStep<BenchStep>(CleanStep::ID);
    setDisplayName(CleanStep::DISPLAY_NAME);
    setSupportedStepList(ProjectExplorer::Constants::BUILDSTEPS_CLEAN);
    setSupportedConfiguration(BuildConfiguration::ID);
    setSupportedProjectType(Project::ID);
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

} // namespace Internal
} // namespace Rust
