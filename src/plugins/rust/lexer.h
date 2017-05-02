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
            RawString,
            OneLineComment,
            MultiLineComment
        };

        State(State::Type type, int depth = 0) : m_type(type), m_depth(depth) {}
        explicit State(int value) : m_type(static_cast<Type>(value & 0x3F)), m_depth(value >> 6) {}
        explicit operator int() { return m_type | (m_depth << 6); }

        void setType(Type type) { m_type = type; }

        Type type() const { return m_type; }

        void setDepth(int depth) { m_depth = depth; }

        int depth() const { return m_depth; }

    private:
        Type m_type;
        int m_depth;
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
