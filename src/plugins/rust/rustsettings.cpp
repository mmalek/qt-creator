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

#include "rustsettings.h"

#include <coreplugin/icore.h>

namespace Rust {
namespace Internal {
namespace Settings {
namespace {

Q_CONSTEXPR QLatin1String GROUP{"Rust"};

} // namespace

QString value(const StringOption& option)
{
    const QSettings *settings = Core::ICore::settings();
    return settings->value(QString("%1/%2").arg(GROUP).arg(option.key),
                           option.defaultValue).toString();
}

void setValue(const StringOption& option, const QString& value)
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(GROUP);
    settings->setValue(option.key, value);
    settings->endGroup();
}

} // namespace Settings
} // namespace Internal
} // namespace Rust
