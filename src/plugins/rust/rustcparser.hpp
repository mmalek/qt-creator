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

#include <projectexplorer/ioutputparser.h>

class QJsonObject;

namespace Utils { class FileName; }

namespace Rust {
namespace Internal {

class RustcParser : public ProjectExplorer::IOutputParser
{
    Q_OBJECT

public:
    void stdOutput(const QString &line) override;

private:
    void parseMessage(const QJsonObject& message);
    void parseCode(const QJsonObject& code, const Utils::FileName &file, int line);

    void showJsonOnConsole(bool value) { m_showJsonOnConsole = value; }
    bool showJsonOnConsole() const { return m_showJsonOnConsole; }

private:
    bool m_showJsonOnConsole = false;
};

} // namespace Internal
} // namespace Rust