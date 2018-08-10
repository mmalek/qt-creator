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

#include <QChar>
#include <QStringView>

#include <array>

namespace Rust {
namespace Internal {

Q_CONSTEXPR QChar CHAR_NUL = 0x0000; // \0
Q_CONSTEXPR QChar CHAR_HT = 0x0009; // \t
Q_CONSTEXPR QChar CHAR_LF = 0x000A; // \n
Q_CONSTEXPR QChar CHAR_CR = 0x000D; // \r
Q_CONSTEXPR QChar CHAR_EXCLAMATION = 0x0021; // !
Q_CONSTEXPR QChar CHAR_DOUBLE_QUOTE = 0x0022; // "
Q_CONSTEXPR QChar CHAR_HASH = 0x0023; // #
Q_CONSTEXPR QChar CHAR_SINGLE_QUOTE = 0x0027; // '
Q_CONSTEXPR QChar CHAR_PARENTHESES_LEFT = 0x0028; // (
Q_CONSTEXPR QChar CHAR_PARENTHESES_RIGHT = 0x0029; // )
Q_CONSTEXPR QChar CHAR_ASTERISK = 0x002A; // *
Q_CONSTEXPR QChar CHAR_PLUS = 0x002B; // +
Q_CONSTEXPR QChar CHAR_COMMA = 0x002C; // ,
Q_CONSTEXPR QChar CHAR_MINUS = 0x002D; // -
Q_CONSTEXPR QChar CHAR_POINT = 0x002E; // .
Q_CONSTEXPR QChar CHAR_SLASH = 0x002F; // /
Q_CONSTEXPR QChar CHAR_0 = 0x0030; // 0
Q_CONSTEXPR QChar CHAR_1 = 0x0031; // 1
Q_CONSTEXPR QChar CHAR_7 = 0x0037; // 7
Q_CONSTEXPR QChar CHAR_9 = 0x0039; // 9
Q_CONSTEXPR QChar CHAR_COLON = 0x003A; // :
Q_CONSTEXPR QChar CHAR_SEMICOLON = 0x003B; // ;
Q_CONSTEXPR QChar CHAR_A_UPPER = 0x0041; // A
Q_CONSTEXPR QChar CHAR_E_UPPER = 0x0045; // E
Q_CONSTEXPR QChar CHAR_F_UPPER = 0x0046; // F
Q_CONSTEXPR QChar CHAR_SQUARE_BRRACKET_LEFT = 0x005B; // [
Q_CONSTEXPR QChar CHAR_SQUARE_BRRACKET_RIGHT = 0x005D; // ]
Q_CONSTEXPR QChar CHAR_BACKSLASH = 0x005C;
Q_CONSTEXPR QChar CHAR_UNDERSCORE = 0x005F; // _
Q_CONSTEXPR QChar CHAR_A_LOWER = 0x0061; // a
Q_CONSTEXPR QChar CHAR_B_LOWER = 0x0062; // b
Q_CONSTEXPR QChar CHAR_E_LOWER = 0x0065; // e
Q_CONSTEXPR QChar CHAR_F_LOWER = 0x0066; // f
Q_CONSTEXPR QChar CHAR_H_LOWER = 0x0068; // f
Q_CONSTEXPR QChar CHAR_N_LOWER = 0x006E; // n
Q_CONSTEXPR QChar CHAR_O_LOWER = 0x006F; // o
Q_CONSTEXPR QChar CHAR_R_LOWER = 0x0072; // r
Q_CONSTEXPR QChar CHAR_T_LOWER = 0x0074; // t
Q_CONSTEXPR QChar CHAR_U_LOWER = 0x0075; // u
Q_CONSTEXPR QChar CHAR_X_LOWER = 0x0078; // x
Q_CONSTEXPR QChar CHAR_BRACE_LEFT = 0x007B; // {
Q_CONSTEXPR QChar CHAR_BRACE_RIGHT = 0x007D; // }

Q_CONSTEXPR QStringView BYTE_START{u"b\'"};
Q_CONSTEXPR QStringView BYTE_STRING_START{u"b\""};
Q_CONSTEXPR QStringView RAW_STRING_START{u"r#"};
Q_CONSTEXPR QStringView RAW_STRING_END{u"\"#"};
Q_CONSTEXPR QStringView RAW_BYTE_STRING_START{u"br#"};
Q_CONSTEXPR QStringView ONE_LINE_COMMENT_START{u"//"};
Q_CONSTEXPR QStringView ONE_LINE_DOC1_COMMENT_START{u"///"};
Q_CONSTEXPR QStringView ONE_LINE_DOC2_COMMENT_START{u"//!"};
Q_CONSTEXPR QStringView MULTI_LINE_COMMENT_START{u"/*"};
Q_CONSTEXPR QStringView MULTI_LINE_DOC1_COMMENT_START{u"/**"};
Q_CONSTEXPR QStringView MULTI_LINE_DOC2_COMMENT_START{u"/*!"};
Q_CONSTEXPR QStringView MULTI_LINE_COMMENT_END{u"*/"};
Q_CONSTEXPR QStringView ATTRIBUTE_START{u"#["};
Q_CONSTEXPR QStringView ATTRIBUTE_NEG_START{u"#!["};
Q_CONSTEXPR QStringView RANGE_OPERATOR{u".."};
Q_CONSTEXPR QStringView PATH_SEPARATOR{u"::"};

Q_CONSTEXPR QStringView KEYWORD_ELSE{u"else"};
Q_CONSTEXPR QStringView KEYWORD_LC_SELF{u"self"};

Q_CONSTEXPR std::array<QStringView, 52> KEYWORDS =
{
    {
        QStringView{u"Self"},
        QStringView{u"abstract"},
        QStringView{u"alignof"},
        QStringView{u"as"},
        QStringView{u"become"},
        QStringView{u"box"},
        QStringView{u"break"},
        QStringView{u"const"},
        QStringView{u"continue"},
        QStringView{u"crate"},
        QStringView{u"do"},
        KEYWORD_ELSE,
        QStringView{u"enum"},
        QStringView{u"extern"},
        QStringView{u"false"},
        QStringView{u"final"},
        QStringView{u"fn"},
        QStringView{u"for"},
        QStringView{u"if"},
        QStringView{u"impl"},
        QStringView{u"in"},
        QStringView{u"let"},
        QStringView{u"loop"},
        QStringView{u"macro"},
        QStringView{u"match"},
        QStringView{u"mod"},
        QStringView{u"move"},
        QStringView{u"mut"},
        QStringView{u"offsetof"},
        QStringView{u"override"},
        QStringView{u"priv"},
        QStringView{u"proc"},
        QStringView{u"pub"},
        QStringView{u"pure"},
        QStringView{u"ref"},
        QStringView{u"return"},
        KEYWORD_LC_SELF,
        QStringView{u"sizeof"},
        QStringView{u"static"},
        QStringView{u"struct"},
        QStringView{u"super"},
        QStringView{u"trait"},
        QStringView{u"true"},
        QStringView{u"type"},
        QStringView{u"typeof"},
        QStringView{u"unsafe"},
        QStringView{u"unsized"},
        QStringView{u"use"},
        QStringView{u"virtual"},
        QStringView{u"where"},
        QStringView{u"while"},
        QStringView{u"yield"},
    }
};

Q_CONSTEXPR std::array<QStringView, 10> INT_TYPES =
{
    {
        QStringView{u"i16"},
        QStringView{u"i32"},
        QStringView{u"i64"},
        QStringView{u"i8"},
        QStringView{u"isize"},
        QStringView{u"u16"},
        QStringView{u"u32"},
        QStringView{u"u64"},
        QStringView{u"u8"},
        QStringView{u"usize"},
    }
};

Q_CONSTEXPR std::array<QStringView, 2> FLOAT_TYPES =
{
    {
        QStringView{u"f32"},
        QStringView{u"f64"},
    }
};

Q_CONSTEXPR std::array<QStringView, 3> OTHER_PRIMITIVE_TYPES =
{
    {
        QStringView{u"bool"},
        QStringView{u"char"},
        QStringView{u"str"},
    }
};

Q_CONSTEXPR std::array<QStringView, 5> STD_TYPES =
{
    {
        QStringView{u"Box"},
        QStringView{u"Option"},
        QStringView{u"Result"},
        QStringView{u"String"},
        QStringView{u"Vec"},
    }
};

Q_CONSTEXPR std::array<QStringView, 4> STD_ENUMS =
{
    {
        QStringView{u"Err"},
        QStringView{u"None"},
        QStringView{u"Ok"},
        QStringView{u"Some"},
    }
};

Q_CONSTEXPR QStringView SHORT_OPERATORS{u"=!?+-*/%&|^<>."};

Q_CONSTEXPR std::array<QStringView, 20> LONG_OPERATORS =
{
    {
        RANGE_OPERATOR,
        QStringView{u"<<"},
        QStringView{u">>"},
        QStringView{u"&&"},
        QStringView{u"||"},
        QStringView{u"=="},
        QStringView{u"!="},
        QStringView{u"<="},
        QStringView{u">="},
        QStringView{u"=>"},
        QStringView{u"+="},
        QStringView{u"-="},
        QStringView{u"*="},
        QStringView{u"/="},
        QStringView{u"%="},
        QStringView{u"&="},
        QStringView{u"|="},
        QStringView{u"^="},
        QStringView{u"<<="},
        QStringView{u">>="},
    }
};

namespace Grammar {

inline bool isXidStart(const QChar c)
{
    return c.isLetter() || c == CHAR_UNDERSCORE;
}

inline bool isXidContinue(const QChar c)
{
    return c.isLetterOrNumber() || c == CHAR_UNDERSCORE;
}

} // namespace Grammar
} // namespace Internal
} // namespace Rust
