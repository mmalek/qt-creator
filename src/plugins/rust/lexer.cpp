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

#include "lexer.h"
#include "token.h"

#include <array>

namespace Rust {
namespace Internal {

namespace {

TokenType toTokenType(Lexer::State state)
{
    switch (state)
    {
    case Lexer::State::Comment:
        return TokenType::Comment;
    case Lexer::State::String:
        return TokenType::String;
    default:
        return TokenType::None;
    }
}

constexpr std::array<const char*, 52> KEYWORDS =
{
    "abstract", "alignof", "as", "become", "box",
    "break", "const", "continue", "crate", "do",
    "else", "enum", "extern", "false", "final",
    "fn", "for", "if", "impl", "in",
    "let", "loop", "macro", "match", "mod",
    "move", "mut", "offsetof", "override", "priv",
    "proc", "pub", "pure", "ref", "return",
    "Self", "self", "sizeof", "static", "struct",
    "super", "trait", "true", "type", "typeof",
    "unsafe", "unsized", "use", "virtual", "where",
    "while", "yield"
};

bool isKeyword(QStringRef text)
{
    return std::any_of(KEYWORDS.begin(), KEYWORDS.end(),
                       [&text](const char* keyword) { return QLatin1String(keyword) == text; });
}

constexpr bool isUnderscore(QChar c)
{
    return c == QLatin1Char('_');
}

constexpr bool isPoint(QChar c)
{
    return c == QLatin1Char('.');
}

constexpr bool isXidStart(QChar c)
{
    return c.isLetter() || isUnderscore(c);
}

constexpr bool isXidContinue(QChar c)
{
    return c.isLetterOrNumber() || isUnderscore(c);
}

constexpr std::array<const char*, 12> NUMSUFFIXES =
{
    "i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64", "isize", "usize", "f32", "f64"
};

bool isNumSuffix(QStringRef text)
{
    return std::any_of(NUMSUFFIXES.begin(), NUMSUFFIXES.end(),
                       [&text](const char* keyword) { return QLatin1String(keyword) == text; });
}

} // namespace

Lexer::Lexer(QStringRef buffer, State state)
    : m_buf(buffer),
      m_pos(0),
      m_state(state)
{
}

Token Lexer::next()
{
    TokenType tokenType = toTokenType(m_state);

    int begin = -1;

    for (; m_pos >= 0 && m_pos < m_buf.size(); ++m_pos) {
        const QChar character = m_buf[m_pos];

        if (tokenType == TokenType::None) {
            if (isXidStart(character)) {
                begin = m_pos;
                tokenType = TokenType::Identifier;
            } else if (character.isNumber()) {
                begin = m_pos;
                tokenType = TokenType::Number;
            }
        } else if (tokenType == TokenType::Identifier) {
            if (!isXidContinue(character)) {
                break;
            }
        } else if (tokenType == TokenType::Number) {
            if (!character.isNumber() && !isUnderscore(character)) {
                const int nextPos = m_pos + 1;
                
                if (isPoint(character) && (nextPos >= m_buf.size() || !isXidStart(m_buf[nextPos]))) {
                    continue;
                } else if (character.isSpace()) {
                    break;
                } else {
                    Lexer lexer(m_buf.mid(m_pos));
                    const Token suffix = lexer.next();
                    if (suffix.begin == 0 && suffix.type == TokenType::Identifier &&
                            isNumSuffix(m_buf.mid(m_pos, suffix.length))) {
                        m_pos += suffix.length;
                    }
                    break;
                }
            }
        }
    }

    if (begin < 0) {
        tokenType = TokenType::None;
    }

    if (tokenType == TokenType::Identifier && isKeyword(m_buf.mid(begin, m_pos - begin))) {
        tokenType = TokenType::Keyword;
    }

    return Token{begin, m_pos - begin, tokenType};
}

} // namespace Internal
} // namespace Rust
