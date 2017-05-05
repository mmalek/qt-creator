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

namespace Rust {
namespace Internal {

enum class TokenType {
    None = 0, // No token found
    Unknown, // Unidentified token found
    Keyword,
    Operator,
    Identifier,
    Char,
    String,
    Number,
    Comment,
    DocComment,
    PrimitiveType,
    Type,
    BraceLeft,
    BraceRight,
    NumTokenTypes
};

struct Token
{
    int begin;
    int length;
    TokenType type;

    bool operator==(const Token &rhs) const
    {
        return begin == rhs.begin && length == rhs.length && type == rhs.type;
    }

    Token() = delete;
};

} // namespace Internal
} // namespace Rust
