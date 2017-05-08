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
#include <tuple>

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
constexpr QChar CHAR_EXCLAMATION = 0x0021; // !
constexpr QChar CHAR_DOUBLE_QUOTE = 0x0022; // "
constexpr QChar CHAR_HASH = 0x0023; // #
constexpr QChar CHAR_SINGLE_QUOTE = 0x0027; // '
constexpr QChar CHAR_PARENTHESES_LEFT = 0x0028; // (
constexpr QChar CHAR_PARENTHESES_RIGHT = 0x0029; // )
constexpr QChar CHAR_ASTERISK = 0x002A; // *
constexpr QChar CHAR_PLUS = 0x002B; // +
constexpr QChar CHAR_POINT = 0x002E; // .
constexpr QChar CHAR_SLASH = 0x002F; // /
constexpr QChar CHAR_0 = 0x0030; // 0
constexpr QChar CHAR_1 = 0x0031; // 1
constexpr QChar CHAR_7 = 0x0037; // 7
constexpr QChar CHAR_9 = 0x0039; // 9
constexpr QChar CHAR_COLON = 0x003A; // :
constexpr QChar CHAR_SEMICOLON = 0x003B; // ;
constexpr QChar CHAR_A_UPPER = 0x0041; // A
constexpr QChar CHAR_E_UPPER = 0x0045; // E
constexpr QChar CHAR_F_UPPER = 0x0046; // F
constexpr QChar CHAR_SQUARE_BRRACKET_LEFT = 0x005B; // [
constexpr QChar CHAR_SQUARE_BRRACKET_RIGHT = 0x005D; // ]
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
constexpr QLatin1String ONE_LINE_DOC1_COMMENT_START{"///"};
constexpr QLatin1String ONE_LINE_DOC2_COMMENT_START{"//!"};
constexpr QLatin1String MULTI_LINE_COMMENT_START{"/*"};
constexpr QLatin1String MULTI_LINE_DOC1_COMMENT_START{"/**"};
constexpr QLatin1String MULTI_LINE_DOC2_COMMENT_START{"/*!"};
constexpr QLatin1String MULTI_LINE_COMMENT_END{"*/"};

constexpr std::array<QLatin1String, 52> KEYWORDS =
{
    QLatin1String("abstract"),
    QLatin1String("alignof"),
    QLatin1String("as"),
    QLatin1String("become"),
    QLatin1String("box"),
    QLatin1String("break"),
    QLatin1String("const"),
    QLatin1String("continue"),
    QLatin1String("crate"),
    QLatin1String("do"),
    QLatin1String("else"),
    QLatin1String("enum"),
    QLatin1String("extern"),
    QLatin1String("false"),
    QLatin1String("final"),
    QLatin1String("fn"),
    QLatin1String("for"),
    QLatin1String("if"),
    QLatin1String("impl"),
    QLatin1String("in"),
    QLatin1String("let"),
    QLatin1String("loop"),
    QLatin1String("macro"),
    QLatin1String("match"),
    QLatin1String("mod"),
    QLatin1String("move"),
    QLatin1String("mut"),
    QLatin1String("offsetof"),
    QLatin1String("override"),
    QLatin1String("priv"),
    QLatin1String("proc"),
    QLatin1String("pub"),
    QLatin1String("pure"),
    QLatin1String("ref"),
    QLatin1String("return"),
    QLatin1String("Self"),
    QLatin1String("self"),
    QLatin1String("sizeof"),
    QLatin1String("static"),
    QLatin1String("struct"),
    QLatin1String("super"),
    QLatin1String("trait"),
    QLatin1String("true"),
    QLatin1String("type"),
    QLatin1String("typeof"),
    QLatin1String("unsafe"),
    QLatin1String("unsized"),
    QLatin1String("use"),
    QLatin1String("virtual"),
    QLatin1String("where"),
    QLatin1String("while"),
    QLatin1String("yield")
};

constexpr std::array<QLatin1String, 10> INT_TYPES =
{
    QLatin1String("i16"),
    QLatin1String("i32"),
    QLatin1String("i64"),
    QLatin1String("i8"),
    QLatin1String("isize"),
    QLatin1String("u16"),
    QLatin1String("u32"),
    QLatin1String("u64"),
    QLatin1String("u8"),
    QLatin1String("usize")
};

constexpr std::array<QLatin1String, 2> FLOAT_TYPES =
{
    QLatin1String("f32"),
    QLatin1String("f64")
};

constexpr std::array<QLatin1String, 3> OTHER_PRIMITIVE_TYPES =
{
    QLatin1String("bool"),
    QLatin1String("char"),
    QLatin1String("str")
};

constexpr std::array<QLatin1String, 5> STD_TYPES =
{
    QLatin1String("Box"),
    QLatin1String("Option"),
    QLatin1String("Result"),
    QLatin1String("String"),
    QLatin1String("Vec")
};

constexpr QLatin1String SHORT_OPERATORS{"=!?+-*/%&|^<>,."};

constexpr std::array<QLatin1String, 20> LONG_OPERATORS =
{
    QLatin1String(".."),
    QLatin1String("<<"),
    QLatin1String(">>"),
    QLatin1String("&&"),
    QLatin1String("||"),
    QLatin1String("=="),
    QLatin1String("!="),
    QLatin1String("<="),
    QLatin1String(">="),
    QLatin1String("=>"),
    QLatin1String("+="),
    QLatin1String("-="),
    QLatin1String("*="),
    QLatin1String("/="),
    QLatin1String("%="),
    QLatin1String("&="),
    QLatin1String("|="),
    QLatin1String("^="),
    QLatin1String("<<="),
    QLatin1String(">>=")
};

template<typename Container>
bool binarySearch(const Container& cont, QStringRef text)
{
    return std::binary_search(cont.begin(), cont.end(), text);
}

bool isKeyword(QStringRef text)
{
    return binarySearch(KEYWORDS, text);
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

bool isIntType(QStringRef text)
{
    return binarySearch(INT_TYPES, text);
}

bool isFloatType(QStringRef text)
{
    return binarySearch(FLOAT_TYPES, text);
}

bool isPrimitiveType(QStringRef text)
{
    return isIntType(text) || isFloatType(text) || binarySearch(OTHER_PRIMITIVE_TYPES, text);
}

bool isStdType(QStringRef text)
{
    return binarySearch(STD_TYPES, text);
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

bool isShortOperator(const QChar c)
{
    for (int i = 0; i < SHORT_OPERATORS.size(); ++i) {
        if (SHORT_OPERATORS[i] == c) {
            return true;
        }
    }

    return false;
}

int isLongOperator(QStringRef text)
{
    for (const QLatin1String& s : LONG_OPERATORS) {
        if (text.startsWith(s)) {
            return s.size();
        }
    }

    return 0;
}

int processChar(QStringRef buf, const QChar quote)
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

std::tuple<Lexer::State, int>
processNumSuffix(QStringRef buf, Lexer::State state)
{
    int pos = 0;

    if (buf[pos] == CHAR_E_UPPER || buf[pos] == CHAR_E_LOWER) {
        ++pos;
        const bool plusPresent = pos < buf.size() && buf[pos] == CHAR_PLUS;

        pos = skipWhile(pos, buf, [](QChar c) { return c.isNumber(); });

        state = plusPresent ? Lexer::State::FloatNumber : Lexer::State::Unknown;
    }

    if (isXidContinue(buf[pos])) {
        const int suffixBegin = pos;
        pos = skipWhile(pos, buf, &isXidContinue);

        if (((state != Lexer::State::Zero && state != Lexer::State::DecNumber) ||
             !isIntType(buf.mid(suffixBegin, pos - suffixBegin))) &&
            (state != Lexer::State::FloatNumber ||
             !isFloatType(buf.mid(suffixBegin, pos - suffixBegin)))) {
            state = Lexer::State::Unknown;
        }
    }

    return std::make_tuple(state, pos);
}

std::tuple<Lexer::State, int, bool>
processBinNumber(QChar character, QStringRef buf, Lexer::State state)
{
    if (isBinDigit(character) || character == CHAR_UNDERSCORE) {
        return std::make_tuple(state, 0, false);
    } else {
        return std::tuple_cat(processNumSuffix(buf, state), std::make_tuple(true));
    }
}

std::tuple<Lexer::State, int, bool>
processHexNumber(QChar character, QStringRef buf, Lexer::State state)
{
    if (isHexDigit(character) || character == CHAR_UNDERSCORE) {
        return std::make_tuple(state, 0, false);
    } else {
        return std::tuple_cat(processNumSuffix(buf, state), std::make_tuple(true));
    }
}

std::tuple<Lexer::State, int, bool>
processOctNumber(QChar character, QStringRef buf, Lexer::State state)
{
    if (isOctDigit(character) || character == CHAR_UNDERSCORE) {
        return std::make_tuple(state, 0, false);
    } else {
        return std::tuple_cat(processNumSuffix(buf, state), std::make_tuple(true));
    }
}

std::tuple<Lexer::State, int, bool>
processDecNumber(QChar character, QStringRef buf, Lexer::State state)
{
    if (character.isNumber() || character == CHAR_UNDERSCORE) {
        if (state == Lexer::State::Zero) {
            state = Lexer::State::DecNumber;
        }
        return std::make_tuple(state, 0, false);
    } else if (character == CHAR_POINT) {
        const int nextPos = 1;
        if (nextPos >= buf.size() || buf[nextPos].isDigit() || buf[nextPos].isSpace()) {
            state = Lexer::State::FloatNumber;
            return std::make_tuple(state, 0, false);
        } else {
            return std::make_tuple(state, 0, true);
        }
    } else {
        return std::tuple_cat(processNumSuffix(buf, state), std::make_tuple(true));
    }
}

std::tuple<Lexer::State, int, bool>
processFloatNumber(QChar character, QStringRef buf, Lexer::State state)
{
    if (character.isNumber() || character == CHAR_UNDERSCORE) {
        return std::make_tuple(state, 0, false);
    } else {
        return std::tuple_cat(processNumSuffix(buf, state), std::make_tuple(true));
    }
}

} // namespace

Lexer::Lexer(QStringRef buffer, State multiLineState, int multiLineDepth)
    : m_buf(buffer),
      m_pos(0),
      m_multiLineState(multiLineState),
      m_multiLineDepth(multiLineDepth)
{
}

Lexer::Lexer(QStringRef buffer, int multiLineState)
    : m_buf(buffer),
      m_pos(0),
      m_multiLineState(multiLineState < 0 ? State::Default
                                          : static_cast<State>(multiLineState & 0x3F)),
      m_multiLineDepth(multiLineState < 0 ? 0 : (multiLineState >> 6))
{
}

Lexer::operator int()
{
    return static_cast<int>(m_multiLineState) | (m_multiLineDepth << 6);
}

Token Lexer::next()
{
    State state = m_multiLineState;

    if (state == State::String &&
            m_pos < m_buf.size() && m_buf[m_pos] == CHAR_BACKSLASH &&
            m_pos+1 < m_buf.size() && isEol(m_buf[m_pos+1])) {
        m_pos = skipWhile(m_pos, m_buf, &isEol);
    }

    int begin = (state != State::Default) ? m_pos : -1;

    for (; m_pos >= 0 && m_pos < m_buf.size(); ++m_pos) {
        const QChar character = m_buf[m_pos];
        const QStringRef slice = m_buf.mid(m_pos);

        if (state == State::Default) {
            if (slice.startsWith(RAW_STRING_START) || slice.startsWith(RAW_BYTE_STRING_START)) {
                begin = m_pos;
                const int posAfterPrefix = skipWhile(m_pos, m_buf,
                    [](const QChar c){ return c == CHAR_B_LOWER || c == CHAR_R_LOWER; });
                m_pos = skipWhile(posAfterPrefix, m_buf, [](const QChar c){ return c == CHAR_HASH; });
                state = State::RawString;
                if (m_pos < m_buf.size() && m_buf[m_pos] == CHAR_DOUBLE_QUOTE) {
                    m_multiLineState = State::RawString;
                    m_multiLineDepth = m_pos - posAfterPrefix;
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
                m_multiLineState = State::String;
                begin = m_pos;
            } else if (slice.startsWith(BYTE_STRING_START)) {
                state = State::String;
                m_multiLineState = State::String;
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
                m_multiLineState = State::MultiLineDocComment;
                m_multiLineDepth = 1;
                begin = m_pos;
            } else if (slice.startsWith(MULTI_LINE_COMMENT_START)) {
                state = State::MultiLineComment;
                m_multiLineState = State::MultiLineComment;
                m_multiLineDepth = 1;
                begin = m_pos;
            } else if (isXidStart(character)) {
                begin = m_pos;
                state = State::IdentOrKeyword;
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
                state = State::ParenthesesLeft;
                break;
            } else if (character == CHAR_PARENTHESES_RIGHT) {
                begin = m_pos;
                ++m_pos;
                state = State::ParenthesesRight;
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
                break;
            } else if (character == CHAR_BRACE_RIGHT) {
                begin = m_pos;
                ++m_pos;
                state = State::BraceRight;
                break;
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
            } else if (!isXidContinue(character)) {
                break;
            }
        } else if (state == State::Zero) {
            if (character == CHAR_B_LOWER)
                state = State::BinNumber;
            else if (character == CHAR_H_LOWER)
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
                m_multiLineState = State::Default;
                break;
            } else if (character == CHAR_BACKSLASH && m_pos+1 < m_buf.size() &&
                       isEol(m_buf[m_pos+1])) {
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
                if (posAfterHashes - m_pos - 1 >= m_multiLineDepth) {
                    m_pos += m_multiLineDepth + 1;
                    m_multiLineState = State::Default;
                    m_multiLineDepth = 0;
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
                ++m_multiLineDepth;
                m_pos += MULTI_LINE_COMMENT_START.size() - 1;
            } else if (slice.startsWith(MULTI_LINE_COMMENT_END)) {
                m_pos += MULTI_LINE_COMMENT_END.size() - 1;
                --m_multiLineDepth;
                if (m_multiLineDepth <= 0) {
                    ++m_pos;
                    m_multiLineState = State::Default;
                    break;
                }
            } else if (isEol(character)) {
                m_pos = skipWhile(m_pos, m_buf, &isEol);
                break;
            }
        }
    }

    TokenType tokenType = [] (const State state, const QStringRef text) {
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
            case State::Colon:
                return TokenType::Colon;
            case State::Semicolon:
                return TokenType::Semicolon;
            case State::ParenthesesLeft:
                return TokenType::ParenthesesLeft;
            case State::ParenthesesRight:
                return TokenType::ParenthesesRight;
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
