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

constexpr bool isUnderscore(const QChar c)
{
    return c.unicode() == 0x005F;
}

constexpr bool isPoint(const QChar c)
{
    return c.unicode() == 0x002E;
}

constexpr bool isXidStart(QChar c)
{
    return c.isLetter() || isUnderscore(c);
}

constexpr bool isXidContinue(QChar c)
{
    return c.isLetterOrNumber() || isUnderscore(c);
}

int skipXidContinue(int pos, QStringRef buf)
{
    do
    {
        ++pos;
    } while (pos < buf.size() && isXidContinue(buf[pos]));
    return pos;
}


constexpr bool isBinDigit(const QChar c)
{
    return c.unicode() == 0x0030 || c.unicode() == 0x0031;
}

constexpr bool isHexDigit(const QChar c)
{
    return (c.unicode() >= 0x0030 && c.unicode() <= 0x0039) ||
           (c.unicode() >= 0x0041 && c.unicode() <= 0x0046) ||
           (c.unicode() >= 0x0061 && c.unicode() <= 0x0066);
}

constexpr bool isOctDigit(const QChar c)
{
    return c.unicode() >= 0x0030 && c.unicode() <= 0x0037;
}

constexpr std::array<const char*, 10> INTSUFFIXES =
{
    "i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64", "isize", "usize"
};

constexpr std::array<const char*, 2> FLOATSUFFIXES =
{
    "f32", "f64"
};

bool isNumSuffix(QStringRef text)
{
    const auto isEqual = [&text](const char* keyword) { return QLatin1String(keyword) == text; };
    return std::any_of(INTSUFFIXES.begin(), INTSUFFIXES.end(), isEqual) ||
           std::any_of(FLOATSUFFIXES.begin(), FLOATSUFFIXES.end(), isEqual);
}

bool isFloatSuffix(QStringRef text)
{
    const auto isEqual = [&text](const char* keyword) { return QLatin1String(keyword) == text; };
    return std::any_of(FLOATSUFFIXES.begin(), FLOATSUFFIXES.end(), isEqual);
}

} // namespace

Lexer::Lexer(QStringRef buffer, State multiLineState)
    : m_buf(buffer),
      m_pos(0),
      m_multiLineState(multiLineState)
{
}

Token Lexer::next()
{
    State state = m_multiLineState;

    int begin = -1;

    auto processNumSuffix = [&](QChar character, bool (*checker)(QStringRef) = &isNumSuffix) {
        if (isXidContinue(character)) {
            const int suffixBegin = m_pos;
            m_pos = skipXidContinue(m_pos, m_buf);

            if (!checker(m_buf.mid(suffixBegin, m_pos - suffixBegin))) {
                state.setType(State::Unknown);
            }
            return true;
        } else {
            return false;
        }
    };

    auto processBinNumber = [&](QChar character) -> bool {
        if (isBinDigit(character) || isUnderscore(character)) {
            return false;
        } else {
            processNumSuffix(character);
            return true;
        }
    };

    auto processHexNumber = [&](QChar character) -> bool {
        if (isHexDigit(character) || isUnderscore(character)) {
            return false;
        } else {
            processNumSuffix(character);
            return true;
        }
    };

    auto processOctNumber = [&](QChar character) -> bool {
        if (isOctDigit(character) || isUnderscore(character)) {
            return false;
        } else {
            processNumSuffix(character);
            return true;
        }
    };

    auto processDecNumber = [&](QChar character) -> bool {
        if (character.isNumber() || isUnderscore(character)) {
            return false;
        } else if (isPoint(character)) {
            const int nextPos = m_pos + 1;
            if (nextPos >= m_buf.size() || m_buf[nextPos].isDigit() || m_buf[nextPos].isSpace()) {
                state.setType(State::FloatNumber);
                return false;
            } else {
                return true;
            }
        } else {
            processNumSuffix(character);
            return true;
        }
    };

    auto processFloatNumber = [&](QChar character) -> bool {
        if (character.isNumber() || isUnderscore(character)) {
            return false;
        } else {
            processNumSuffix(character, &isFloatSuffix);
            return true;
        }
    };

    for (; m_pos >= 0 && m_pos < m_buf.size(); ++m_pos) {
        const QChar character = m_buf[m_pos];

        if (state.type() == State::Default) {
            if (isXidStart(character)) {
                begin = m_pos;
                state.setType(State::IdentOrKeyword);
            } else if (character.isNumber()) {
                begin = m_pos;
                state.setType(character.digitValue() == 0 ? State::Zero : State::DecNumber);
            }
        } else if (state.type() == State::IdentOrKeyword) {
            if (!isXidContinue(character)) {
                break;
            }
        } else if (state.type() == State::Zero) {
            if (character.unicode() == 0x0062)
                state.setType(State::BinNumber);
            else if (character.unicode() == 0x0068)
                state.setType(State::HexNumber);
            else if (character.unicode() == 0x006F)
                state.setType(State::OctNumber);
            else if (processDecNumber(character))
                break;
        } else if (state.type() == State::BinNumber) {
            if (processBinNumber(character))
                break;
        } else if (state.type() == State::HexNumber) {
            if (processHexNumber(character))
                break;
        } else if (state.type() == State::OctNumber) {
            if (processOctNumber(character))
                break;
        } else if (state.type() == State::DecNumber) {
            if (processDecNumber(character))
                break;
        } else if (state.type() == State::FloatNumber) {
            if (processFloatNumber(character))
                break;
        }
    }

    TokenType tokenType;

    if (begin < 0) {
        tokenType = TokenType::None;
    } else {
        switch (state.type()) {
        case State::IdentOrKeyword:
            tokenType = isKeyword(m_buf.mid(begin, m_pos - begin)) ? TokenType::Keyword
                                                                   : TokenType::Identifier;
            break;
        case State::Zero:
        case State::BinNumber:
        case State::DecNumber:
        case State::HexNumber:
        case State::OctNumber:
        case State::FloatNumber:
            tokenType = TokenType::Number;
            break;
        case State::String:
            tokenType = TokenType::String;
            break;
        case State::Comment:
            tokenType = TokenType::Comment;
            break;
        case State::Unknown:
            tokenType = TokenType::Unknown;
            break;
        default:
            tokenType = TokenType::None;
            break;
        }
    }

    switch (state.type()) {
    case State::String:
        m_multiLineState.setType(State::String);
        break;
    case State::Comment:
        m_multiLineState.setType(State::Comment);
        m_multiLineState.setCommentDepth(state.commentDepth());
        break;
    default:
        m_multiLineState.setType(State::Default);
        break;
    }

    return Token{begin, m_pos - begin, tokenType};
}

} // namespace Internal
} // namespace Rust
