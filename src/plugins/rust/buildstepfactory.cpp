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

#include "buildstepfactory.h"
#include "buildstep.h"

#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <utils/qtcassert.h>

#include <QScopedPointer>

namespace Rust {

BuildStepFactory::BuildStepFactory(QObject *parent)
    : ProjectExplorer::IBuildStepFactory(parent)
{
}

QList<ProjectExplorer::BuildStepInfo> BuildStepFactory::availableSteps(ProjectExplorer::BuildStepList *parent) const
{
    if (parent->id() == ProjectExplorer::Constants::BUILDSTEPS_BUILD) {
        return {{ BuildStep::ID, tr(BuildStep::DISPLAY_NAME) }};
    } else if (parent->id() == ProjectExplorer::Constants::BUILDSTEPS_CLEAN) {
        return {{ CleanStep::ID, tr(CleanStep::DISPLAY_NAME) }};
    } else {
        return {};
    }
}

ProjectExplorer::BuildStep *BuildStepFactory::create(ProjectExplorer::BuildStepList *parent, Core::Id id)
{
    if (id == BuildStep::ID) {
        return new BuildStep(parent);
    } else if (id == CleanStep::ID) {
        return new CleanStep(parent);
    } else {
        return nullptr;
    }
}

ProjectExplorer::BuildStep *BuildStepFactory::clone(ProjectExplorer::BuildStepList *parent, ProjectExplorer::BuildStep *product)
{
    QTC_ASSERT(parent, return nullptr);
    QTC_ASSERT(product, return nullptr);
    QScopedPointer<BuildStep> result(new BuildStep(parent));
    return result->fromMap(product->toMap()) ? result.take() : nullptr;
}

} // namespace Rust
