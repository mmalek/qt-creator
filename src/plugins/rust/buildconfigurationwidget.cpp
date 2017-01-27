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

#include "buildconfigurationwidget.h"
#include "buildconfiguration.h"
#include "ui_buildconfigurationwidget.h"

namespace Rust {

namespace {
constexpr int BUILD_TYPE_DEBUG = 0;
constexpr int BUILD_TYPE_RELEASE = 1;
}

BuildConfigurationWidget::BuildConfigurationWidget(BuildConfiguration *buildConfiguration)
    : m_ui(new Ui::BuildConfigurationWidget)
{
    m_ui->setupUi(this);

    if (buildConfiguration->buildType() == BuildConfiguration::Debug) {
        m_ui->buildVariant->setCurrentIndex(BUILD_TYPE_DEBUG);
    } else if (buildConfiguration->buildType() == BuildConfiguration::Release) {
        m_ui->buildVariant->setCurrentIndex(BUILD_TYPE_RELEASE);
    }

    connect(m_ui->buildVariant,
            static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [buildConfiguration](int index) {
        if (index == BUILD_TYPE_DEBUG) {
            buildConfiguration->setBuildType(BuildConfiguration::Debug);
        } else if (index == BUILD_TYPE_RELEASE) {
            buildConfiguration->setBuildType(BuildConfiguration::Release);
        }
    });
}

BuildConfigurationWidget::~BuildConfigurationWidget()
{
}

} // namespace Rust
