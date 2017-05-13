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
#include <utils/pathchooser.h>
#include <utils/utilsicons.h>

#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>

namespace Rust {
namespace Internal {

namespace {
    QString toString(Product::Kind kind)
    {
        switch(kind)
        {
        case Product::Benchmark: return QLatin1String("ben");
        case Product::Binary: return QLatin1String("bin");
        case Product::Example: return QLatin1String("exa");
        case Product::Test: return QLatin1String("tes");
        case Product::Library: return QLatin1String("lib");
        default: Q_UNREACHABLE();
        }
    }

    Product::Kind toProductKind(const QStringRef& kind)
    {
        if (kind == "ben") {
            return Product::Benchmark;
        } else if (kind == "bin") {
            return Product::Binary;
        } else if (kind == "exa") {
            return Product::Example;
        } else if (kind == "tes") {
            return Product::Test;
        } else if (kind == "lib") {
            return Product::Library;
        } else {
            Q_UNREACHABLE();
        }
    }

    constexpr char RC_PREFIX[] = "Rust.RunConfiguration";

    Core::Id toRunConfigurationId(const Product& product)
    {
        return Core::Id::fromString(QString("%1.%2.%3")
                                        .arg(RC_PREFIX)
                                        .arg(toString(product.kind))
                                        .arg(product.name));
    }

    const Product* toProduct(const Project& project, Core::Id id)
    {
        const QString suffix = id.suffixAfter(RC_PREFIX);
        Q_ASSERT(suffix.length() > 5);
        const Product::Kind kind = toProductKind(suffix.midRef(1, 3));
        const QStringRef name = suffix.midRef(5);

        for (const Product& product : project.products()) {
            if (product.kind == kind && product.name == name) {
                return &product;
            }
        }
        return nullptr;
    }

    QPair<Product::Kind,QString> getKindAndName(Core::Id id)
    {
        const QString suffix = id.suffixAfter(RC_PREFIX);
        Q_ASSERT(suffix.length() > 5);
        const Product::Kind kind = toProductKind(suffix.midRef(1, 3));
        const QString name = suffix.mid(5);

        return qMakePair(kind, name);
    }

    QString getProductDisplayName(Core::Id id)
    {
        return getKindAndName(id).second;
    }
} // namespace

RunConfiguration::RunConfiguration(ProjectExplorer::Target *parent, Core::Id id)
    : ProjectExplorer::RunConfiguration(parent, id)
{
    addExtraAspect(new ProjectExplorer::ArgumentsAspect(
                       this, QStringLiteral("Rust.RunConfiguration.CommandLineArguments")));
    addExtraAspect(new ProjectExplorer::WorkingDirectoryAspect(
                       this, QStringLiteral("Rust.RunConfiguration.WorkingDirectory")));

    addExtraAspect(new ProjectExplorer::TerminalAspect(
                       this, QStringLiteral("Rust.RunConfiguration.UseTerminal")));

    setDefaultDisplayName(defaultDisplayName());
}

RunConfiguration::RunConfiguration(ProjectExplorer::Target *parent, RunConfiguration *source)
    : ProjectExplorer::RunConfiguration(parent, source)
{
    setDefaultDisplayName(defaultDisplayName());
}

bool RunConfiguration::isEnabled() const
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

Utils::FileName RunConfiguration::executable(ProjectExplorer::BuildConfiguration* bc) const
{
    if (const Product* p = product()) {
        Utils::FileName path = bc->buildDirectory();

        switch(p->kind) {
        case Product::Binary:
            path.appendPath(p->name);
            break;
        case Product::Example:
            path.appendPath(QLatin1String("examples")).appendPath(p->name);
            break;
        default:
            Q_UNIMPLEMENTED();
            break;
        }

        return path;
    } else {
        return Utils::FileName();
    }
}

Utils::FileName RunConfiguration::workingDirectory() const
{
    if (const Product* p = product()) {
        Utils::FileName path = target()->project()->projectDirectory();

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

const Project& RunConfiguration::project() const
{
    Project* project = qobject_cast<Project*>(target()->project());
    Q_ASSERT(project != nullptr);
    return *project;
}

const Product* RunConfiguration::product() const
{
    return toProduct(project(), id());
}

QString RunConfiguration::defaultDisplayName() const
{
    QString defaultName = getProductDisplayName(id());
    return !defaultName.isEmpty() ? defaultName : tr("Rust Run Configuration");
}

RunConfigurationFactory::RunConfigurationFactory(QObject *parent)
    : IRunConfigurationFactory(parent)
{
}

QList<Core::Id> RunConfigurationFactory::availableCreationIds(ProjectExplorer::Target *parent,
                                                              CreationMode mode) const
{
    Q_UNUSED(mode);

    QList<Core::Id> ids;

    if (Project *project = qobject_cast<Project *>(parent->project())) {
        for (const Product& product : project->products()) {
            if (product.kind == Product::Binary || product.kind == Product::Example)
                ids << toRunConfigurationId(product);
        }
    }

    return ids;
}

QString RunConfigurationFactory::displayNameForId(Core::Id id) const
{
    return getProductDisplayName(id);
}

bool RunConfigurationFactory::canCreate(ProjectExplorer::Target *parent, Core::Id id) const
{
    Project *project = qobject_cast<Project *>(parent->project());
    return project && toProduct(*project, id);
}

bool RunConfigurationFactory::canRestore(ProjectExplorer::Target *parent,
                                         const QVariantMap &map) const
{
    Q_UNUSED(parent);
    return ProjectExplorer::idFromMap(map).toString().startsWith(QLatin1String(RC_PREFIX));
}

bool RunConfigurationFactory::canClone(ProjectExplorer::Target *parent,
                                       ProjectExplorer::RunConfiguration *source) const
{
    return canCreate(parent, source->id());
}

ProjectExplorer::RunConfiguration *RunConfigurationFactory::clone(
        ProjectExplorer::Target *parent, ProjectExplorer::RunConfiguration *source)
{
    if (canClone(parent, source)) {
        RunConfiguration *old = qobject_cast<RunConfiguration *>(source);
        return new RunConfiguration(parent, old);
    } else {
        return 0;
    }
}

ProjectExplorer::RunConfiguration *RunConfigurationFactory::doCreate(
        ProjectExplorer::Target *parent, Core::Id id)
{
    return new RunConfiguration(parent, id);
}

ProjectExplorer::RunConfiguration *RunConfigurationFactory::doRestore(
        ProjectExplorer::Target *parent, const QVariantMap &map)
{
    return new RunConfiguration(parent, ProjectExplorer::idFromMap(map));
}

RunConfigurationWidget::RunConfigurationWidget(RunConfiguration *rc)
    : m_rc(rc)
{
    auto vboxTopLayout = new QVBoxLayout(this);
    vboxTopLayout->setMargin(0);

    auto hl = new QHBoxLayout();
    hl->addStretch();
    m_disabledIcon = new QLabel(this);
    m_disabledIcon->setPixmap(Utils::Icons::WARNING.pixmap());
    hl->addWidget(m_disabledIcon);
    m_disabledReason = new QLabel(this);
    m_disabledReason->setVisible(false);
    hl->addWidget(m_disabledReason);
    hl->addStretch();
    vboxTopLayout->addLayout(hl);

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

    runConfigurationEnabledChange();

    connect(m_rc->target(), &ProjectExplorer::Target::activeBuildConfigurationChanged,
            this, &RunConfigurationWidget::onActiveBuildConfigChanged);

    connect(m_rc, &RunConfiguration::enabledChanged,
            this, &RunConfigurationWidget::runConfigurationEnabledChange);
}

void RunConfigurationWidget::onActiveBuildConfigChanged(ProjectExplorer::BuildConfiguration *bc)
{
    const QString text = m_rc->executable(bc).toString();
    m_executableLineLabel->setText(text.isEmpty() ? tr("<unknown>") : text);
}

void RunConfigurationWidget::runConfigurationEnabledChange()
{
    bool enabled = m_rc->isEnabled();
    m_disabledIcon->setVisible(!enabled);
    m_disabledReason->setVisible(!enabled);
    m_disabledReason->setText(m_rc->disabledReason());

    onActiveBuildConfigChanged(m_rc->target()->activeBuildConfiguration());
}

} // namespace Internal
} // namespace Rust
