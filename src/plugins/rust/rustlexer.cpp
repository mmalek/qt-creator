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

#include "rustlexer.h"
#include "rustgrammar.h"
#include "rusttoken.h"

#include <algorithm>
#include <tuple>

namespace Rust {
namespace Internal {

namespace {

enum class State {
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
    MultiLineComment,
    OneLineDocComment,
    MultiLineDocComment,
    Comma,
    Colon,
    Semicolon,
    ParenthesisLeft,
    ParenthesisRight,
    SquareBracketLeft,
    SquareBracketRight,
    BraceLeft,
    BraceRight,
    Operator,
    Attribute,
};

State toState(Lexer::MultiLineState multiLineState, quint8 multiLineParam)
{
    switch (multiLineState) {
    case Lexer::MultiLineState::Comment:
        return State::MultiLineComment;
    case Lexer::MultiLineState::DocComment:
        return State::MultiLineDocComment;
    case Lexer::MultiLineState::String:
        return (multiLineParam == 0) ? State::String : State::RawString;
    default:
        return State::Default;
    }
}

enum class CharState {
    Start,
    EscapeStart,
    End
};

template<typename Container>
bool contains(const Container& cont, QStringView text)
{
    return std::any_of(
                cont.begin(),
                cont.end(),
                [&text](const QStringView &element) {
        return std::equal(element.cbegin(), element.end(), text.cbegin(), text.cend());
    });
}

bool isKeyword(QStringView text)
{
    return contains(KEYWORDS, text);
}

int skipWhile(int pos, QStringView buf, bool (*predicate)(QChar))
{
    do
    {
        ++pos;
    } while (pos < buf.size() && predicate(buf[pos]));
    return pos;
}

bool isBinDigit(const QChar c)
{
    return c == CHAR_0 || c == CHAR_1;
}

bool isHexDigit(const QChar c)
{
    return (c >= CHAR_0 && c <= CHAR_9) ||
           (c >= CHAR_A_UPPER && c <= CHAR_F_UPPER) ||
           (c >= CHAR_A_LOWER && c <= CHAR_F_LOWER);
}

bool isOctDigit(const QChar c)
{
    return c >= CHAR_0 && c <= CHAR_7;
}

bool isIntType(QStringView text)
{
    return contains(INT_TYPES, text);
}

bool isFloatType(QStringView text)
{
    return contains(FLOAT_TYPES, text);
}

bool isPrimitiveType(QStringView text)
{
    return isIntType(text) || isFloatType(text) || contains(OTHER_PRIMITIVE_TYPES, text);
}

bool isStdType(QStringView text)
{
    return contains(STD_TYPES, text);
}

bool isStdEnum(QStringView text)
{
    return contains(STD_ENUMS, text);
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

bool isEol(const QChar c)
{
    return c == CHAR_LF || c == CHAR_CR;
}

bool isUnprintableChar(const QChar c)
{
    return isEol(c) || c == CHAR_NUL || c == CHAR_HT;
}

bool isShortOperator(const QChar c)
{
    for (int i = 0; i < SHORT_OPERATORS.size(); ++i) {
        if (SHORT_OPERATORS[i] == c) {
            return true;
        }
    }

    return false;
}

int isLongOperator(QStringView text)
{
    for (const auto& s : LONG_OPERATORS) {
        if (text.startsWith(s)) {
            return static_cast<int>(s.size());
        }
    }

    return 0;
}

int processChar(QStringView buf, const QChar quote)
{
    CharState chState = CharState::Start;

    int pos = 0;
    int begin = pos;

    for (; pos < buf.size() && chState != CharState::End; ++pos) {
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
}

std::tuple<State, int>
processNumSuffix(QStringView buf, State state)
{
    int pos = 0;

    if (buf[pos] == CHAR_E_UPPER || buf[pos] == CHAR_E_LOWER) {
        ++pos;
        const bool signPresent = pos < buf.size() &&
                                 (buf[pos] == CHAR_PLUS || buf[pos] == CHAR_MINUS);

        pos = skipWhile(pos, buf, [](QChar c) { return c.isNumber(); });

        state = signPresent ? State::FloatNumber : State::Unknown;
    }

    if (Grammar::isXidContinue(buf[pos])) {
        const int suffixBegin = pos;
        pos = skipWhile(pos, buf, &Grammar::isXidContinue);

        if (((state != State::Zero && state != State::DecNumber) ||
             !isIntType(buf.mid(suffixBegin, pos - suffixBegin))) &&
            (state != State::FloatNumber ||
             !isFloatType(buf.mid(suffixBegin, pos - suffixBegin)))) {
            state = State::Unknown;
        }
    }

    return std::make_tuple(state, pos);
}

std::tuple<State, int, bool>
processBinNumber(QChar character, QStringView buf, State state)
{
    if (isBinDigit(character) || character == CHAR_UNDERSCORE) {
        return std::make_tuple(state, 0, false);
    } else {
        return std::tuple_cat(processNumSuffix(buf, state), std::make_tuple(true));
    }
}

std::tuple<State, int, bool>
processHexNumber(QChar character, QStringView buf, State state)
{
    if (isHexDigit(character) || character == CHAR_UNDERSCORE) {
        return std::make_tuple(state, 0, false);
    } else {
        return std::tuple_cat(processNumSuffix(buf, state), std::make_tuple(true));
    }
}

std::tuple<State, int, bool>
processOctNumber(QChar character, QStringView buf, State state)
{
    if (isOctDigit(character) || character == CHAR_UNDERSCORE) {
        return std::make_tuple(state, 0, false);
    } else {
        return std::tuple_cat(processNumSuffix(buf, state), std::make_tuple(true));
    }
}

std::tuple<State, int, bool>
processDecNumber(QChar character, QStringView buf, State state)
{
    if (character.isNumber() || character == CHAR_UNDERSCORE) {
        if (state == State::Zero) {
            state = State::DecNumber;
        }
        return std::make_tuple(state, 0, false);
    } else if (character == CHAR_POINT) {
        const int nextPos = 1;
        if (nextPos >= buf.size() ||
                (!Grammar::isXidStart(buf[nextPos]) && buf[nextPos] != CHAR_POINT)) {
            state = State::FloatNumber;
            return std::make_tuple(state, 0, false);
        } else {
            return std::make_tuple(state, 0, true);
        }
    } else {
        return std::tuple_cat(processNumSuffix(buf, state), std::make_tuple(true));
    }
}

std::tuple<State, int, bool>
processFloatNumber(QChar character, QStringView buf, State state)
{
    if (character.isNumber() || character == CHAR_UNDERSCORE) {
        return std::make_tuple(state, 0, false);
    } else {
        return std::tuple_cat(processNumSuffix(buf, state), std::make_tuple(true));
    }
}

} // namespace

Token Lexer::next()
{
    State state = toState(m_multiLineState, m_multiLineParam);

    if (state == State::String &&
            m_pos < m_buf.size() && m_buf[m_pos] == CHAR_BACKSLASH &&
            m_pos+1 < m_buf.size() && isEol(m_buf[m_pos+1])) {
        m_pos = skipWhile(m_pos, m_buf, &isEol);
    }

    int begin = (state != State::Default) ? m_pos : 0;

    for (; m_pos >= 0 && m_pos < m_buf.size(); ++m_pos) {
        const QChar character = m_buf[m_pos];
        const QStringView slice = m_buf.mid(m_pos);

        if (state == State::Default) {
            if (slice.startsWith(RAW_STRING_START) || slice.startsWith(RAW_BYTE_STRING_START)) {
                begin = m_pos;
                const int posAfterPrefix = skipWhile(m_pos, m_buf,
                    [](const QChar c){ return c == CHAR_B_LOWER || c == CHAR_R_LOWER; });
                m_pos = skipWhile(posAfterPrefix, m_buf, [](const QChar c){ return c == CHAR_HASH; });
                state = State::RawString;
                if (m_pos < m_buf.size() && m_buf[m_pos] == CHAR_DOUBLE_QUOTE) {
                    m_multiLineState = MultiLineState::String;
                    m_multiLineParam = static_cast<quint8>(m_pos - posAfterPrefix);
                    ++m_depth;
                }
            } else if (character.isNumber()) {
                begin = m_pos;
                state = (character.digitValue() == 0) ? State::Zero : State::DecNumber;
            } else if (character == CHAR_SINGLE_QUOTE || slice.startsWith(BYTE_START)) {
                const int prefix = (character == CHAR_B_LOWER) ? 2 : 1;
                const int chars = processChar(m_buf.mid(m_pos + prefix), CHAR_SINGLE_QUOTE);
                const int endPos = m_pos + chars + prefix;

                if (chars > 0 && endPos < m_buf.size() && m_buf[endPos] == CHAR_SINGLE_QUOTE) {
                    state = State::Char;
                    begin = m_pos;
                    m_pos += chars + prefix + 1;
                    break;
                }
            } else if (character == CHAR_DOUBLE_QUOTE) {
                state = State::String;
                m_multiLineState = MultiLineState::String;
                m_multiLineParam = 0;
                ++m_depth;
                begin = m_pos;
            } else if (slice.startsWith(BYTE_STRING_START)) {
                state = State::String;
                m_multiLineState = MultiLineState::String;
                m_multiLineParam = 0;
                ++m_depth;
                begin = m_pos;
                ++m_pos;
            } else if (slice.startsWith(ONE_LINE_DOC1_COMMENT_START) ||
                       slice.startsWith(ONE_LINE_DOC2_COMMENT_START)) {
                state = State::OneLineDocComment;
                begin = m_pos;
            } else if (slice.startsWith(ONE_LINE_COMMENT_START)) {
                state = State::OneLineComment;
                begin = m_pos;
            } else if (slice.startsWith(MULTI_LINE_DOC1_COMMENT_START) ||
                       slice.startsWith(MULTI_LINE_DOC2_COMMENT_START)) {
                state = State::MultiLineDocComment;
                m_multiLineState = MultiLineState::DocComment;
                m_multiLineParam = 1;
                ++m_depth;
                begin = m_pos;
            } else if (slice.startsWith(MULTI_LINE_COMMENT_START)) {
                state = State::MultiLineComment;
                m_multiLineState = MultiLineState::Comment;
                m_multiLineParam = 1;
                ++m_depth;
                begin = m_pos;
            } else if (Grammar::isXidStart(character)) {
                begin = m_pos;
                state = State::IdentOrKeyword;
            } else if (character == CHAR_COMMA) {
                begin = m_pos;
                ++m_pos;
                state = State::Comma;
                break;
            } else if (character == CHAR_COLON) {
                begin = m_pos;
                ++m_pos;
                state = State::Colon;
                break;
            } else if (character == CHAR_SEMICOLON) {
                begin = m_pos;
                ++m_pos;
                state = State::Semicolon;
                break;
            } else if (character == CHAR_PARENTHESES_LEFT) {
                begin = m_pos;
                ++m_pos;
                state = State::ParenthesisLeft;
                break;
            } else if (character == CHAR_PARENTHESES_RIGHT) {
                begin = m_pos;
                ++m_pos;
                state = State::ParenthesisRight;
                break;
            } else if (character == CHAR_SQUARE_BRRACKET_LEFT) {
                begin = m_pos;
                ++m_pos;
                state = State::SquareBracketLeft;
                break;
            } else if (character == CHAR_SQUARE_BRRACKET_RIGHT) {
                begin = m_pos;
                ++m_pos;
                state = State::SquareBracketRight;
                break;
            } else if (character == CHAR_BRACE_LEFT) {
                begin = m_pos;
                ++m_pos;
                state = State::BraceLeft;
                ++m_depth;
                break;
            } else if (character == CHAR_BRACE_RIGHT) {
                begin = m_pos;
                ++m_pos;
                state = State::BraceRight;
                if (m_depth > 0) {
                    --m_depth;
                }
                break;
            } else if (slice.startsWith(ATTRIBUTE_START) || slice.startsWith(ATTRIBUTE_NEG_START)) {
                begin = m_pos;
                state = State::Attribute;
            } else if (const int size = isLongOperator(slice)) {
                begin = m_pos;
                m_pos += size;
                state = State::Operator;
                break;
            } else if (isShortOperator(character)) {
                begin = m_pos;
                ++m_pos;
                state = State::Operator;
                break;
            } else if (!character.isSpace()) {
                begin = m_pos;
                ++m_pos;
                state = State::Unknown;
                break;
            }
        } else if (state == State::IdentOrKeyword) {
            if (character == CHAR_EXCLAMATION) {
                ++m_pos;
                break;
            } else if (!Grammar::isXidContinue(character)) {
                break;
            }
        } else if (state == State::Zero) {
            if (character == CHAR_B_LOWER)
                state = State::BinNumber;
            else if (character == CHAR_X_LOWER)
                state = State::HexNumber;
            else if (character == CHAR_O_LOWER)
                state = State::OctNumber;
            else {
                const auto ret = processDecNumber(character, slice, state);
                state = std::get<0>(ret);
                m_pos += std::get<1>(ret);
                if (std::get<2>(ret)) {
                    break;
                }
            }
        } else if (state == State::BinNumber) {
            const auto ret = processBinNumber(character, slice, state);
            state = std::get<0>(ret);
            m_pos += std::get<1>(ret);
            if (std::get<2>(ret)) {
                break;
            }
        } else if (state == State::HexNumber) {
            const auto ret = processHexNumber(character, slice, state);
            state = std::get<0>(ret);
            m_pos += std::get<1>(ret);
            if (std::get<2>(ret)) {
                break;
            }
        } else if (state == State::OctNumber) {
            const auto ret = processOctNumber(character, slice, state);
            state = std::get<0>(ret);
            m_pos += std::get<1>(ret);
            if (std::get<2>(ret)) {
                break;
            }
        } else if (state == State::DecNumber) {
            const auto ret = processDecNumber(character, slice, state);
            state = std::get<0>(ret);
            m_pos += std::get<1>(ret);
            if (std::get<2>(ret)) {
                break;
            }
        } else if (state == State::FloatNumber) {
            const auto ret = processFloatNumber(character, slice, state);
            state = std::get<0>(ret);
            m_pos += std::get<1>(ret);
            if (std::get<2>(ret)) {
                break;
            }
        } else if (state == State::String) {
            if (character == CHAR_DOUBLE_QUOTE) {
                ++m_pos;
                m_multiLineState = MultiLineState::Default;
                m_multiLineParam = 0;
                --m_depth;
                break;
            } else if (character == CHAR_BACKSLASH &&
                       (m_pos+1 >= m_buf.size() || isEol(m_buf[m_pos+1]))) {
                m_pos = skipWhile(m_pos, m_buf, &isEol);
                break;
            } else if (isEol(character)) {
                m_pos = skipWhile(m_pos, m_buf, &isEol);
                break;
            } else {
                const int chars = processChar(m_buf.mid(m_pos), CHAR_DOUBLE_QUOTE);
                if (chars > 0) {
                    m_pos += chars - 1;
                } else {
                    state = State::Unknown;
                    break;
                }
            }
        } else if (state == State::RawString) {
            if (slice.startsWith(RAW_STRING_END)) {
                const int posAfterHashes = skipWhile(m_pos + 1, m_buf,
                                                     [](const QChar c){ return c == CHAR_HASH; });
                if (posAfterHashes - m_pos - 1 >= m_multiLineParam) {
                    m_pos += m_multiLineParam + 1;
                    m_multiLineState = MultiLineState::Default;
                    m_multiLineParam = 0;
                    --m_depth;
                    break;
                }
            } else if (isEol(character)) {
                m_pos = skipWhile(m_pos, m_buf, &isEol);
                break;
            }
        } else if (state == State::OneLineComment || state == State::OneLineDocComment) {
            if (isEol(character)) {
                m_pos = skipWhile(m_pos, m_buf, &isEol);
                break;
            }
        } else if (state == State::MultiLineComment || state == State::MultiLineDocComment) {
            if (slice.startsWith(MULTI_LINE_COMMENT_START)) {
                ++m_multiLineParam;
                ++m_depth;
                m_pos += MULTI_LINE_COMMENT_START.size() - 1;
            } else if (slice.startsWith(MULTI_LINE_COMMENT_END)) {
                m_pos += MULTI_LINE_COMMENT_END.size() - 1;
                --m_multiLineParam;
                if (m_depth > 0) {
                    --m_depth;
                }
                if (m_multiLineParam == 0) {
                    ++m_pos;
                    m_multiLineState = MultiLineState::Default;
                    m_multiLineParam = 0;
                    break;
                }
            } else if (isEol(character)) {
                m_pos = skipWhile(m_pos, m_buf, &isEol);
                break;
            }
        } else if (state == State::Attribute) {
            if (character == CHAR_SQUARE_BRRACKET_RIGHT) {
                ++m_pos;
                break;
            }
        }
    }

    TokenType tokenType = [] (const State state, const QStringView text) {
        if (text.isEmpty()) {
            return TokenType::None;
        } else {
            switch (state) {
            case State::IdentOrKeyword:
                if (isKeyword(text)) {
                    return TokenType::Keyword;
                } else if (isPrimitiveType(text)) {
                    return TokenType::PrimitiveType;
                } else if (isStdType(text)) {
                    return TokenType::Type;
                } else if (isStdEnum(text)) {
                    return TokenType::Enumeration;
                } else {
                    return TokenType::Identifier;
                }
            case State::Zero:
            case State::BinNumber:
            case State::DecNumber:
            case State::HexNumber:
            case State::OctNumber:
            case State::FloatNumber:
                return TokenType::Number;
            case State::Char:
                return TokenType::Char;
            case State::String:
            case State::RawString:
                return TokenType::String;
            case State::MultiLineComment:
            case State::OneLineComment:
                return TokenType::Comment;
            case State::MultiLineDocComment:
            case State::OneLineDocComment:
                return TokenType::DocComment;
            case State::Comma:
                return TokenType::Comma;
            case State::Colon:
                return TokenType::Colon;
            case State::Semicolon:
                return TokenType::Semicolon;
            case State::ParenthesisLeft:
                return TokenType::ParenthesisLeft;
            case State::ParenthesisRight:
                return TokenType::ParenthesisRight;
            case State::SquareBracketLeft:
                return TokenType::SquareBracketLeft;
            case State::SquareBracketRight:
                return TokenType::SquareBracketRight;
            case State::BraceLeft:
                return TokenType::BraceLeft;
            case State::BraceRight:
                return TokenType::BraceRight;
            case State::Operator:
                return TokenType::Operator;
            case State::Attribute:
                return TokenType::Attribute;
            case State::Unknown:
                return TokenType::Unknown;
            default:
                return TokenType::None;
            }
        }
    } (state, m_buf.mid(begin, m_pos - begin));

    return Token{begin, m_pos - begin, tokenType};
}

} // namespace Internal
} // namespace Rust
