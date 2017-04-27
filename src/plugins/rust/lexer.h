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

#include <QStringMatcher>
#include <QStringRef>

namespace Rust {
namespace Internal {

struct Token;

class Lexer final
{
public:
    class State {
    public:
        enum Type
        {
            Default = 0,
            Unknown = 1,
            IdentOrKeyword,
            Zero,
            BinNumber,
            DecNumber,
            HexNumber,
            OctNumber,
            FloatNumber,
            Char,
            String,
            Comment = 50
        };

        State(State::Type type) : m_value(type) {}
        explicit State(int value) : m_value(value) {}
        explicit operator int() { return m_value; }

        void setType(Type type) { m_value = type; }

        Type type() const { return m_value < Comment ? static_cast<Type>(m_value) : Comment; }

        void setCommentDepth(int depth) { m_value = (depth <= 0) ? Default : Comment + depth; }

        int commentDepth() const { return m_value >= Comment ? (m_value - Comment + 1) : 0; }

    private:
        int m_value;
    };

public:
    explicit Lexer(QStringRef buffer, State multiLineState = State::Default);

    State multiLineState() const { return m_multiLineState; }

    Token next();

private:
    QStringRef m_buf;
    int m_pos;
    State m_multiLineState;
};

} // namespace Internal
} // namespace Rust
