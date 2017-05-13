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

#include <QStringRef>
#include <QtGlobal>

namespace Rust {
namespace Internal {

struct Token;

class Lexer final
{
public:
    enum class MultiLineState : quint8 {
        Default,
        Comment,
        DocComment,
        String
    };

public:
    explicit Lexer(QStringRef buffer,
                   MultiLineState multiLineState = MultiLineState::Default,
                   quint8 multiLineParam = 0,
                   int depth = 0)
        : m_buf(buffer),
          m_pos(0),
          m_multiLineState(multiLineState),
          m_multiLineParam(multiLineParam),
          m_depth(depth)
    {}

    MultiLineState multiLineState() const { return m_multiLineState; }

    quint8 multiLineParam() const { return m_multiLineParam; }

    int depth() const { return m_depth; }

    Token next();

private:
    QStringRef m_buf;
    int m_pos;
    MultiLineState m_multiLineState;
    quint8 m_multiLineParam;
    int m_depth;
};

} // namespace Internal
} // namespace Rust
