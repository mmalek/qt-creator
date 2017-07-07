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

#include <QString>

#ifdef Q_OS_WIN
#define MAKE_BINARY_NAME(name) name ".exe"
#else
#define MAKE_BINARY_NAME(name) name
#endif

namespace Rust {
namespace Internal {
namespace Settings {

struct StringOption
{
    QLatin1String key;
    QString defaultValue;
};

static const StringOption CARGO {QLatin1String{"Cargo"}, QLatin1String{MAKE_BINARY_NAME("cargo")}};
static const StringOption RUSTUP {QLatin1String{"Rustup"}, QLatin1String{MAKE_BINARY_NAME("rustup")}};
static const StringOption RACER {QLatin1String{"Racer"}, QLatin1String{MAKE_BINARY_NAME("racer")}};

QString value(const StringOption& option);
void setValue(const StringOption& option, const QString& value);

} // namespace Settings
} // namespace Internal
} // namespace Rust
