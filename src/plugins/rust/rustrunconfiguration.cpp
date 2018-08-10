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

#include "rustrunconfiguration.h"
#include "rustproject.h"

#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/runconfigurationaspects.h>
#include <projectexplorer/runnables.h>
#include <projectexplorer/target.h>
#include <utils/detailswidget.h>
#include <utils/fileutils.h>
#include <utils/hostosinfo.h>
#include <utils/pathchooser.h>
#include <utils/utilsicons.h>

#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QRegularExpression>

namespace Rust {
namespace Internal {

namespace {
    QStringView toStringView(Product::Kind kind)
    {
        switch(kind)
        {
        case Product::Benchmark: return u"ben";
        case Product::Binary: return u"bin";
        case Product::Example: return u"exa";
        case Product::Test: return u"tes";
        case Product::Library: return u"lib";
        default: Q_UNREACHABLE();
        }
    }

    Product::Kind toProductKind(const QStringView& kind)
    {
        if (kind == u"ben") {
            return Product::Benchmark;
        } else if (kind == u"bin") {
            return Product::Binary;
        } else if (kind == u"exa") {
            return Product::Example;
        } else if (kind == u"tes") {
            return Product::Test;
        } else if (kind == u"lib") {
            return Product::Library;
        } else {
            Q_UNREACHABLE();
        }
    }

    constexpr char RC_PREFIX[] = "Rust.RunConfiguration";

    QString toRunConfigurationExtra(const Product& product)
    {
        return QStringLiteral("%1.%2")
                .arg(toStringView(product.kind))
                .arg(product.name);
    }

    QPair<Product::Kind, QString> getKindAndName(const QString &extra)
    {
        QRegularExpression re(QStringLiteral("^(?<kind>\\w+)\\.(?<name>.+)"));
        auto match = re.match(extra);
        if (match.hasMatch()) {
            auto kind = match.captured(u"kind");
            auto name = match.captured(u"name");
            return {toProductKind(kind), name};
        } else {
            return {};
        }
    }

    const Product* toProduct(const Project& project, const QString &extra)
    {
        auto kindAndName = getKindAndName(extra);
        const Product::Kind kind = kindAndName.first;
        const QString &name = kindAndName.second;

        if (!name.isEmpty()) {
            for (const Product& product : project.products()) {
                if (product.kind == kind && product.name == name) {
                    return &product;
                }
            }
        }
        return nullptr;
    }

} // namespace

RunConfiguration::RunConfiguration(ProjectExplorer::Target *parent)
    : ProjectExplorer::RunConfiguration (parent, RC_PREFIX)
{
    addExtraAspect(new ProjectExplorer::ArgumentsAspect(
                       this, QStringLiteral("Rust.RunConfiguration.CommandLineArguments")));
    addExtraAspect(new ProjectExplorer::WorkingDirectoryAspect(
                       this, QStringLiteral("Rust.RunConfiguration.WorkingDirectory")));

    addExtraAspect(new ProjectExplorer::TerminalAspect(
                       this, QStringLiteral("Rust.RunConfiguration.UseTerminal")));

    setDefaultDisplayName(defaultDisplayName());
}

bool RunConfiguration::isConfigured() const
{
    return product() != nullptr;
}

QWidget *RunConfiguration::createConfigurationWidget()
{
    return new RunConfigurationWidget(this);
}

ProjectExplorer::Runnable RunConfiguration::runnable() const
{
    ProjectExplorer::StandardRunnable r;
    r.executable = executable(activeBuildConfiguration()).toString();
    r.workingDirectory = extraAspect<ProjectExplorer::WorkingDirectoryAspect>()->workingDirectory().toString();
    r.commandLineArguments = extraAspect<ProjectExplorer::ArgumentsAspect>()->arguments();
    r.runMode = extraAspect<ProjectExplorer::TerminalAspect>()->runMode();
    return r;
}

bool RunConfiguration::fromMap(const QVariantMap &map)
{
    if (!ProjectExplorer::RunConfiguration::fromMap(map))
        return false;

    QString extraId = ProjectExplorer::idFromMap(map).suffixAfter(id());
    m_product = toProduct(rustProject(), extraId);
    setDefaultDisplayName(defaultDisplayName());

    return true;
}

Utils::FileName RunConfiguration::executable(ProjectExplorer::BuildConfiguration* bc) const
{
    if (const Product* p = product()) {
        Utils::FileName path = bc->buildDirectory();

        QString name = p->name;
        if (Utils::HostOsInfo::isWindowsHost()) {
            name.append(QLatin1String(".exe"));
        }

        switch(p->kind) {
        case Product::Binary:
            return path.appendPath(name);
        case Product::Example:
            return path.appendPath(QLatin1String("examples")).appendPath(name);
        default:
            Q_UNIMPLEMENTED();
            return {};
        }
    } else {
        return {};
    }
}

Utils::FileName RunConfiguration::workingDirectory() const
{
    if (const Product* p = product()) {
        Utils::FileName path = project()->projectDirectory();

        switch(p->kind) {
        case Product::Example:
            path.appendPath(QLatin1String("examples"));
            break;
        case Product::Test:
            Q_UNIMPLEMENTED();
            break;
        default:
            break;
        }

        return path;
    } else {
        return Utils::FileName();
    }
}

const Project& RunConfiguration::rustProject() const
{
    Project* p = qobject_cast<Project*>(project());
    Q_ASSERT(p != nullptr);
    return *p;
}

QString RunConfiguration::defaultDisplayName() const
{
    return m_product ? m_product->name : tr("Rust Run Configuration");
}

RunConfigurationFactory::RunConfigurationFactory(QObject *parent)
    : IRunConfigurationFactory(parent)
{
    registerRunConfiguration<RunConfiguration>(RC_PREFIX);
    addSupportedProjectType(Project::ID);
    setSupportedTargetDeviceTypes({ProjectExplorer::Constants::DESKTOP_DEVICE_TYPE});
}

QList<ProjectExplorer::RunConfigurationCreationInfo> RunConfigurationFactory::availableCreators(
        ProjectExplorer::Target *parent) const
{
    QList<ProjectExplorer::RunConfigurationCreationInfo> creators;

    if (Project *project = qobject_cast<Project *>(parent->project())) {
        for (const Product& product : project->products()) {
            if (product.kind == Product::Binary || product.kind == Product::Example) {
                creators.push_back(
                            ProjectExplorer::RunConfigurationCreationInfo {
                                this,
                                RC_PREFIX,
                                toRunConfigurationExtra(product),
                                product.name});
            }
        }
    }

    return creators;
}

bool RunConfigurationFactory::canHandle(ProjectExplorer::Target *target) const
{
    return ProjectExplorer::IRunConfigurationFactory::canHandle(target);
}

RunConfigurationWidget::RunConfigurationWidget(RunConfiguration *rc)
    : m_rc(rc)
{
    auto vboxTopLayout = new QVBoxLayout(this);
    vboxTopLayout->setMargin(0);

    auto detailsContainer = new Utils::DetailsWidget(this);
    detailsContainer->setState(Utils::DetailsWidget::NoSummary);
    vboxTopLayout->addWidget(detailsContainer);
    auto detailsWidget = new QWidget(detailsContainer);
    detailsContainer->setWidget(detailsWidget);
    auto toplayout = new QFormLayout(detailsWidget);
    toplayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    toplayout->setMargin(0);

    m_executableLineLabel = new QLabel(this);
    m_executableLineLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    toplayout->addRow(tr("Executable:"), m_executableLineLabel);

    m_rc->extraAspect<ProjectExplorer::ArgumentsAspect>()->addToMainConfigurationWidget(this, toplayout);
    m_rc->extraAspect<ProjectExplorer::WorkingDirectoryAspect>()->addToMainConfigurationWidget(this, toplayout);
    m_rc->extraAspect<ProjectExplorer::TerminalAspect>()->addToMainConfigurationWidget(this, toplayout);

    ProjectExplorer::WorkingDirectoryAspect *aspect = m_rc->extraAspect<ProjectExplorer::WorkingDirectoryAspect>();
    aspect->setDefaultWorkingDirectory(m_rc->workingDirectory());
    aspect->pathChooser()->setBaseFileName(m_rc->target()->project()->projectDirectory());

    connect(m_rc->target(), &ProjectExplorer::Target::activeBuildConfigurationChanged,
            this, &RunConfigurationWidget::onActiveBuildConfigChanged);

    connect(m_rc, &RunConfiguration::enabledChanged,
            this, &RunConfigurationWidget::runConfigurationEnabledChange);
    runConfigurationEnabledChange();
}

void RunConfigurationWidget::onActiveBuildConfigChanged(ProjectExplorer::BuildConfiguration *bc)
{
    const QString text = m_rc->executable(bc).toString();
    m_executableLineLabel->setText(text.isEmpty() ? tr("<unknown>") : text);
}

void RunConfigurationWidget::runConfigurationEnabledChange()
{
    onActiveBuildConfigChanged(m_rc->target()->activeBuildConfiguration());
}

} // namespace Internal
} // namespace Rust
