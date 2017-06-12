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

#include <coreplugin/id.h>
#include <utils/fileutils.h>

#include <QObject>
#include <QVector>

namespace Rust {
namespace Internal {

struct ToolChain
{
    Core::Id id;

    QString name;
    QString toolChainName;
    Utils::FileName path;
    QString version;
    QString racerVersion;
    bool autodetected;

    bool fromRustup() const { return !toolChainName.isNull(); }
    bool isRacerPresent() const { return !racerVersion.isNull(); }
};

class ToolChainManager : public QObject
{
    Q_OBJECT
public:
    explicit ToolChainManager(QObject *parent = nullptr);

    const QVector<ToolChain>& toolChains() const { return m_toolChains; }

signals:

public slots:

private:
    QVector<ToolChain> m_toolChains;
};

} // namespace Internal
} // namespace Rust
