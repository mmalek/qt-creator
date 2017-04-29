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

#include <algorithm>
#include <array>

namespace Rust {
namespace Internal {

namespace {

enum class CharState {
    Start,
    EscapeStart,
    End
};

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

int skipWhile(int pos, QStringRef buf, bool (*predicate)(QChar))
{
    do
    {
        ++pos;
    } while (pos < buf.size() && predicate(buf[pos]));
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

bool isIntSuffix(QStringRef text)
{
    const auto isEqual = [&text](const char* keyword) { return QLatin1String(keyword) == text; };
    return std::any_of(INTSUFFIXES.begin(), INTSUFFIXES.end(), isEqual);
}

constexpr std::array<const char*, 2> FLOATSUFFIXES =
{
    "f32", "f64"
};

bool isFloatSuffix(QStringRef text)
{
    const auto isEqual = [&text](const char* keyword) { return QLatin1String(keyword) == text; };
    return std::any_of(FLOATSUFFIXES.begin(), FLOATSUFFIXES.end(), isEqual);
}

constexpr std::array<QChar, 7> ESCAPED_CHARS =
{
    0x0022, 0x0027, 0x0030, 0x005C, 0x006E, 0x0072, 0x0074
};

bool isEscapedChar(const QChar c)
{
    return std::binary_search(ESCAPED_CHARS.begin(), ESCAPED_CHARS.end(), c);
}

constexpr std::array<QChar, 4> UNPRINTABLE_CHARS =
{
    0x0000, 0x0009, 0x000A, 0x000D
};

bool isUnprintableChar(const QChar c)
{
    return std::binary_search(UNPRINTABLE_CHARS.begin(), UNPRINTABLE_CHARS.end(), c.unicode());
}

int processChar(int pos, QStringRef buf, const QChar quote)
{
    CharState chState = CharState::Start;

    int begin = pos;

    for (; pos >= 0 && pos < buf.size() && chState != CharState::End; ++pos) {
        const QChar character = buf[pos];
        if (isUnprintableChar(character)) {
            break;
        } else if (chState == CharState::Start) {
            if (character.unicode() == 0x005C) {
                chState = CharState::EscapeStart;
            } else if (character == quote) {
                break;
            } else {
                chState = CharState::End;
            }
        } else if (chState == CharState::EscapeStart) {
            if (character.unicode() == 0x0078) {
                const int posAfterHexDigits = skipWhile(pos + 1, buf, &isHexDigit);
                if (posAfterHexDigits - pos == 3) {
                    pos = posAfterHexDigits - 1;
                    chState = CharState::End;
                } else {
                    break;
                }
            } else if (character.unicode() == 0x0075) {
                ++pos;
                if (pos >= buf.size() || buf[pos].unicode() != 0x007B) {
                    break;
                }

                const int posAfterHexDigits = skipWhile(pos, buf, &isHexDigit);
                const int numberOfDigits = posAfterHexDigits - pos;
                if (numberOfDigits < 1 || numberOfDigits > 6) {
                    break;
                }

                pos = posAfterHexDigits;
                if (pos >= buf.size() || buf[pos].unicode() != 0x007D) {
                    break;
                }

                chState = CharState::End;
            } else if (isEscapedChar(character)) {
                chState = CharState::End;
            } else {
                break;
            }
        }
    }

    return (chState == CharState::End) ? pos : begin;
};

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

    auto processNumSuffix = [&]() {
        bool shouldBreak = false;

        if (m_buf[m_pos].unicode() == 0x0045 || m_buf[m_pos].unicode() == 0x0065) {
            ++m_pos;
            const bool plusPresent = m_pos < m_buf.size() && m_buf[m_pos].unicode() == 0x002B;

            m_pos = skipWhile(m_pos, m_buf, [](QChar c) { return c.isNumber(); });

            state.setType(plusPresent ? State::FloatNumber : State::Unknown);
            shouldBreak = true;
        }

        if (isXidContinue(m_buf[m_pos])) {
            const int suffixBegin = m_pos;
            m_pos = skipWhile(m_pos, m_buf, &isXidContinue);

            if (((state.type() != State::Zero && state.type() != State::DecNumber) ||
                 !isIntSuffix(m_buf.mid(suffixBegin, m_pos - suffixBegin))) &&
                (state.type() != State::FloatNumber ||
                 !isFloatSuffix(m_buf.mid(suffixBegin, m_pos - suffixBegin)))) {
                state.setType(State::Unknown);
            }

            shouldBreak = true;
        }

        return shouldBreak;
    };

    auto processBinNumber = [&](QChar character) -> bool {
        if (isBinDigit(character) || isUnderscore(character)) {
            return false;
        } else {
            processNumSuffix();
            return true;
        }
    };

    auto processHexNumber = [&](QChar character) -> bool {
        if (isHexDigit(character) || isUnderscore(character)) {
            return false;
        } else {
            processNumSuffix();
            return true;
        }
    };

    auto processOctNumber = [&](QChar character) -> bool {
        if (isOctDigit(character) || isUnderscore(character)) {
            return false;
        } else {
            processNumSuffix();
            return true;
        }
    };

    auto processDecNumber = [&](QChar character) -> bool {
        if (character.isNumber() || isUnderscore(character)) {
            if (state.type() == State::Zero) {
                state.setType(State::DecNumber);
            }
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
            processNumSuffix();
            return true;
        }
    };

    auto processFloatNumber = [&](QChar character) -> bool {
        if (character.isNumber() || isUnderscore(character)) {
            return false;
        } else {
            processNumSuffix();
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
            } else if (character.unicode() == 0x0027) {
                const int posAfterChar = processChar(m_pos+1, m_buf, QChar(0x0027));

                begin = m_pos;
                if (posAfterChar - m_pos - 1 > 0 && posAfterChar < m_buf.size() &&
                        m_buf[posAfterChar].unicode() == 0x0027) {
                    state.setType(State::Char);
                    m_pos = posAfterChar + 1;
                    break;
                }
            } else if (character.unicode() == 0x0022) {
                state.setType(State::String);
                begin = m_pos;
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
        } else if (state.type() == State::String) {
            if (character.unicode() == 0x0022) {
                ++m_pos;
                break;
            } else {
                const int posAfterChar = processChar(m_pos, m_buf, QChar(0x0022));
                if (posAfterChar - m_pos > 0) {
                    m_pos = posAfterChar - 1;
                } else {
                    state.setType(State::Unknown);
                    break;
                }
            }
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
        case State::Char:
            tokenType = TokenType::Char;
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

    return Token{begin, m_pos - begin, tokenType};
}

} // namespace Internal
} // namespace Rust
