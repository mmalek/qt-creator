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

constexpr QChar CHAR_NUL = 0x0000; // \0
constexpr QChar CHAR_HT = 0x0009; // \t
constexpr QChar CHAR_LF = 0x000A; // \n
constexpr QChar CHAR_CR = 0x000D; // \r
constexpr QChar CHAR_DOUBLE_QUOTE = 0x0022; // "
constexpr QChar CHAR_HASH = 0x0023; // #
constexpr QChar CHAR_SINGLE_QUOTE = 0x0027; // '
constexpr QChar CHAR_PLUS = 0x002B; // +
constexpr QChar CHAR_POINT = 0x002E; // .
constexpr QChar CHAR_0 = 0x0030; // 0
constexpr QChar CHAR_1 = 0x0031; // 1
constexpr QChar CHAR_7 = 0x0037; // 7
constexpr QChar CHAR_9 = 0x0039; // 9
constexpr QChar CHAR_A_UPPER = 0x0041; // A
constexpr QChar CHAR_E_UPPER = 0x0045; // E
constexpr QChar CHAR_F_UPPER = 0x0046; // F
constexpr QChar CHAR_BACKSLASH = 0x005C;
constexpr QChar CHAR_UNDERSCORE = 0x005F; // _
constexpr QChar CHAR_A_LOWER = 0x0061; // a
constexpr QChar CHAR_B_LOWER = 0x0062; // b
constexpr QChar CHAR_E_LOWER = 0x0065; // e
constexpr QChar CHAR_F_LOWER = 0x0066; // f
constexpr QChar CHAR_H_LOWER = 0x0068; // f
constexpr QChar CHAR_N_LOWER = 0x006E; // n
constexpr QChar CHAR_O_LOWER = 0x006F; // o
constexpr QChar CHAR_R_LOWER = 0x0072; // r
constexpr QChar CHAR_T_LOWER = 0x0074; // t
constexpr QChar CHAR_U_LOWER = 0x0075; // u
constexpr QChar CHAR_X_LOWER = 0x0078; // x
constexpr QChar CHAR_BRACE_LEFT = 0x007B; // {
constexpr QChar CHAR_BRACE_RIGHT = 0x007D; // }

constexpr QLatin1String BYTE_START{"b\'"};
constexpr QLatin1String BYTE_STRING_START{"b\""};
constexpr QLatin1String RAW_STRING_START{"r#"};
constexpr QLatin1String RAW_STRING_END{"\"#"};
constexpr QLatin1String RAW_BYTE_STRING_START{"br#"};
constexpr QLatin1String ONE_LINE_COMMENT_START{"//"};
constexpr QLatin1String MULTI_LINE_COMMENT_START{"/*"};
constexpr QLatin1String MULTI_LINE_COMMENT_END{"*/"};

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

constexpr bool isXidStart(const QChar c)
{
    return c.isLetter() || c == CHAR_UNDERSCORE;
}

constexpr bool isXidContinue(QChar c)
{
    return c.isLetterOrNumber() || c == CHAR_UNDERSCORE;
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
    return c == CHAR_0 || c == CHAR_1;
}

constexpr bool isHexDigit(const QChar c)
{
    return (c >= CHAR_0 && c <= CHAR_9) ||
           (c >= CHAR_A_UPPER && c <= CHAR_F_UPPER) ||
           (c >= CHAR_A_LOWER && c <= CHAR_F_LOWER);
}

constexpr bool isOctDigit(const QChar c)
{
    return c >= CHAR_0 && c <= CHAR_7;
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

bool isEscapedChar(const QChar c)
{
    return
        c == CHAR_DOUBLE_QUOTE ||
        c == CHAR_SINGLE_QUOTE ||
        c == CHAR_0 ||
        c == CHAR_BACKSLASH ||
        c == CHAR_N_LOWER ||
        c == CHAR_R_LOWER ||
        c == CHAR_T_LOWER;
}

constexpr bool isEol(const QChar c)
{
    return c == CHAR_LF || c == CHAR_CR;
}

constexpr bool isUnprintableChar(const QChar c)
{
    return isEol(c) || c == CHAR_NUL || c == CHAR_HT;
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
            if (character == CHAR_BACKSLASH) {
                chState = CharState::EscapeStart;
            } else if (character == quote) {
                break;
            } else {
                chState = CharState::End;
            }
        } else if (chState == CharState::EscapeStart) {
            if (character == CHAR_X_LOWER) {
                const int posAfterHexDigits = skipWhile(pos + 1, buf, &isHexDigit);
                if (posAfterHexDigits - pos == 3) {
                    pos = posAfterHexDigits - 1;
                    chState = CharState::End;
                } else {
                    break;
                }
            } else if (character == CHAR_U_LOWER) {
                ++pos;
                if (pos >= buf.size() || buf[pos] != CHAR_BRACE_LEFT) {
                    break;
                }

                const int posAfterHexDigits = skipWhile(pos, buf, &isHexDigit);
                const int numberOfDigits = posAfterHexDigits - pos;
                if (numberOfDigits < 1 || numberOfDigits > 6) {
                    break;
                }

                pos = posAfterHexDigits;
                if (pos >= buf.size() || buf[pos] != CHAR_BRACE_RIGHT) {
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

    if (state.type() == State::String &&
            m_pos < m_buf.size() && m_buf[m_pos] == CHAR_BACKSLASH &&
            m_pos+1 < m_buf.size() && isEol(m_buf[m_pos+1])) {
        m_pos = skipWhile(m_pos, m_buf, &isEol);
    }

    int begin = (state.type() != State::Default) ? m_pos : -1;

    auto processNumSuffix = [&]() {
        bool shouldBreak = false;

        if (m_buf[m_pos] == CHAR_E_UPPER || m_buf[m_pos] == CHAR_E_LOWER) {
            ++m_pos;
            const bool plusPresent = m_pos < m_buf.size() && m_buf[m_pos] == CHAR_PLUS;

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
        if (isBinDigit(character) || character == CHAR_UNDERSCORE) {
            return false;
        } else {
            processNumSuffix();
            return true;
        }
    };

    auto processHexNumber = [&](QChar character) -> bool {
        if (isHexDigit(character) || character == CHAR_UNDERSCORE) {
            return false;
        } else {
            processNumSuffix();
            return true;
        }
    };

    auto processOctNumber = [&](QChar character) -> bool {
        if (isOctDigit(character) || character == CHAR_UNDERSCORE) {
            return false;
        } else {
            processNumSuffix();
            return true;
        }
    };

    auto processDecNumber = [&](QChar character) -> bool {
        if (character.isNumber() || character == CHAR_UNDERSCORE) {
            if (state.type() == State::Zero) {
                state.setType(State::DecNumber);
            }
            return false;
        } else if (character == CHAR_POINT) {
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
        if (character.isNumber() || character == CHAR_UNDERSCORE) {
            return false;
        } else {
            processNumSuffix();
            return true;
        }
    };

    for (; m_pos >= 0 && m_pos < m_buf.size(); ++m_pos) {
        const QChar character = m_buf[m_pos];
        const QStringRef slice = m_buf.mid(m_pos);

        if (state.type() == State::Default) {
            if (slice.startsWith(RAW_STRING_START) || slice.startsWith(RAW_BYTE_STRING_START)) {
                begin = m_pos;
                const int posAfterPrefix = skipWhile(m_pos, m_buf,
                    [](const QChar c){ return c == CHAR_B_LOWER || c == CHAR_R_LOWER; });
                m_pos = skipWhile(posAfterPrefix, m_buf, [](const QChar c){ return c == CHAR_HASH; });
                if (m_buf[m_pos] == CHAR_DOUBLE_QUOTE) {
                    state.setType(State::RawString);
                    m_multiLineState.setType(State::RawString);
                    m_multiLineState.setDepth(m_pos - posAfterPrefix);
                } else {
                    state.setType(State::Unknown);
                }
            } else if (character.isNumber()) {
                begin = m_pos;
                state.setType(character.digitValue() == 0 ? State::Zero : State::DecNumber);
            } else if (character == CHAR_SINGLE_QUOTE || slice.startsWith(BYTE_START)) {
                begin = m_pos;

                if (character == CHAR_B_LOWER) {
                    ++m_pos;
                }

                const int posAfterChar = processChar(m_pos+1, m_buf, CHAR_SINGLE_QUOTE);

                if (posAfterChar - m_pos - 1 > 0 && posAfterChar < m_buf.size() &&
                        m_buf[posAfterChar] == CHAR_SINGLE_QUOTE) {
                    state.setType(State::Char);
                    m_pos = posAfterChar + 1;
                    break;
                }
            } else if (character == CHAR_DOUBLE_QUOTE) {
                state.setType(State::String);
                m_multiLineState.setType(State::String);
                begin = m_pos;
            } else if (slice.startsWith(BYTE_STRING_START)) {
                state.setType(State::String);
                m_multiLineState.setType(State::String);
                begin = m_pos;
                ++m_pos;
            } else if (slice.startsWith(ONE_LINE_COMMENT_START)) {
                state.setType(State::OneLineComment);
                begin = m_pos;
            } else if (slice.startsWith(MULTI_LINE_COMMENT_START)) {
                state.setType(State::MultiLineComment);
                m_multiLineState.setType(State::MultiLineComment);
                m_multiLineState.setDepth(1);
                begin = m_pos;
            } else if (isXidStart(character)) {
                begin = m_pos;
                state.setType(State::IdentOrKeyword);
            }
        } else if (state.type() == State::IdentOrKeyword) {
            if (!isXidContinue(character)) {
                break;
            }
        } else if (state.type() == State::Zero) {
            if (character == CHAR_B_LOWER)
                state.setType(State::BinNumber);
            else if (character == CHAR_H_LOWER)
                state.setType(State::HexNumber);
            else if (character == CHAR_O_LOWER)
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
            if (character == CHAR_DOUBLE_QUOTE) {
                ++m_pos;
                m_multiLineState.setType(State::Default);
                break;
            } else if (character == CHAR_BACKSLASH && m_pos+1 < m_buf.size() &&
                       isEol(m_buf[m_pos+1])) {
                break;
            } else if (isEol(character)) {
                m_pos = skipWhile(m_pos, m_buf, &isEol);
                break;
            } else {
                const int posAfterChar = processChar(m_pos, m_buf, CHAR_DOUBLE_QUOTE);
                if (posAfterChar - m_pos > 0) {
                    m_pos = posAfterChar - 1;
                } else {
                    state.setType(State::Unknown);
                    break;
                }
            }
        } else if (state.type() == State::RawString) {
            if (slice.startsWith(RAW_STRING_END)) {
                const int posAfterHashes = skipWhile(m_pos + 1, m_buf,
                                                     [](const QChar c){ return c == CHAR_HASH; });
                if (posAfterHashes - m_pos - 1 >= m_multiLineState.depth()) {
                    m_pos += m_multiLineState.depth() + 1;
                    m_multiLineState.setType(State::Default);
                    m_multiLineState.setDepth(0);
                    break;
                }
            } else if (isEol(character)) {
                m_pos = skipWhile(m_pos, m_buf, &isEol);
                break;
            }
        } else if (state.type() == State::OneLineComment) {
            if (isEol(character)) {
                m_pos = skipWhile(m_pos, m_buf, &isEol);
                break;
            }
        } else if (state.type() == State::MultiLineComment) {
            if (slice.startsWith(MULTI_LINE_COMMENT_START)) {
                m_multiLineState.setDepth(m_multiLineState.depth() + 1);
                m_pos += MULTI_LINE_COMMENT_START.size() - 1;
            } else if (slice.startsWith(MULTI_LINE_COMMENT_END)) {
                m_pos += MULTI_LINE_COMMENT_END.size() - 1;
                m_multiLineState.setDepth(m_multiLineState.depth() - 1);
                if (m_multiLineState.depth() <= 0) {
                    ++m_pos;
                    m_multiLineState.setType(State::Default);
                    break;
                }
            } else if (isEol(character)) {
                m_pos = skipWhile(m_pos, m_buf, &isEol);
                break;
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
        case State::RawString:
            tokenType = TokenType::String;
            if (m_pos == begin && m_multiLineState.type() == State::String) {
                m_multiLineState.setType(State::Default);
            }
            break;
        case State::MultiLineComment:
        case State::OneLineComment:
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
