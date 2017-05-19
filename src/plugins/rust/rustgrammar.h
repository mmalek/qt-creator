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
#include <QLatin1String>

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

Q_CONSTEXPR QLatin1String BYTE_START{"b\'"};
Q_CONSTEXPR QLatin1String BYTE_STRING_START{"b\""};
Q_CONSTEXPR QLatin1String RAW_STRING_START{"r#"};
Q_CONSTEXPR QLatin1String RAW_STRING_END{"\"#"};
Q_CONSTEXPR QLatin1String RAW_BYTE_STRING_START{"br#"};
Q_CONSTEXPR QLatin1String ONE_LINE_COMMENT_START{"//"};
Q_CONSTEXPR QLatin1String ONE_LINE_DOC1_COMMENT_START{"///"};
Q_CONSTEXPR QLatin1String ONE_LINE_DOC2_COMMENT_START{"//!"};
Q_CONSTEXPR QLatin1String MULTI_LINE_COMMENT_START{"/*"};
Q_CONSTEXPR QLatin1String MULTI_LINE_DOC1_COMMENT_START{"/**"};
Q_CONSTEXPR QLatin1String MULTI_LINE_DOC2_COMMENT_START{"/*!"};
Q_CONSTEXPR QLatin1String MULTI_LINE_COMMENT_END{"*/"};
Q_CONSTEXPR QLatin1String ATTRIBUTE_START{"#["};

Q_CONSTEXPR std::array<QLatin1String, 52> KEYWORDS =
{
    QLatin1String{"Self"},
    QLatin1String{"abstract"},
    QLatin1String{"alignof"},
    QLatin1String{"as"},
    QLatin1String{"become"},
    QLatin1String{"box"},
    QLatin1String{"break"},
    QLatin1String{"const"},
    QLatin1String{"continue"},
    QLatin1String{"crate"},
    QLatin1String{"do"},
    QLatin1String{"else"},
    QLatin1String{"enum"},
    QLatin1String{"extern"},
    QLatin1String{"false"},
    QLatin1String{"final"},
    QLatin1String{"fn"},
    QLatin1String{"for"},
    QLatin1String{"if"},
    QLatin1String{"impl"},
    QLatin1String{"in"},
    QLatin1String{"let"},
    QLatin1String{"loop"},
    QLatin1String{"macro"},
    QLatin1String{"match"},
    QLatin1String{"mod"},
    QLatin1String{"move"},
    QLatin1String{"mut"},
    QLatin1String{"offsetof"},
    QLatin1String{"override"},
    QLatin1String{"priv"},
    QLatin1String{"proc"},
    QLatin1String{"pub"},
    QLatin1String{"pure"},
    QLatin1String{"ref"},
    QLatin1String{"return"},
    QLatin1String{"self"},
    QLatin1String{"sizeof"},
    QLatin1String{"static"},
    QLatin1String{"struct"},
    QLatin1String{"super"},
    QLatin1String{"trait"},
    QLatin1String{"true"},
    QLatin1String{"type"},
    QLatin1String{"typeof"},
    QLatin1String{"unsafe"},
    QLatin1String{"unsized"},
    QLatin1String{"use"},
    QLatin1String{"virtual"},
    QLatin1String{"where"},
    QLatin1String{"while"},
    QLatin1String{"yield"}
};

Q_CONSTEXPR std::array<QLatin1String, 10> INT_TYPES =
{
    QLatin1String{"i16"},
    QLatin1String{"i32"},
    QLatin1String{"i64"},
    QLatin1String{"i8"},
    QLatin1String{"isize"},
    QLatin1String{"u16"},
    QLatin1String{"u32"},
    QLatin1String{"u64"},
    QLatin1String{"u8"},
    QLatin1String{"usize"}
};

Q_CONSTEXPR std::array<QLatin1String, 2> FLOAT_TYPES =
{
    QLatin1String{"f32"},
    QLatin1String{"f64"}
};

Q_CONSTEXPR std::array<QLatin1String, 3> OTHER_PRIMITIVE_TYPES =
{
    QLatin1String{"bool"},
    QLatin1String{"char"},
    QLatin1String{"str"}
};

Q_CONSTEXPR std::array<QLatin1String, 5> STD_TYPES =
{
    QLatin1String{"Box"},
    QLatin1String{"Option"},
    QLatin1String{"Result"},
    QLatin1String{"String"},
    QLatin1String{"Vec"}
};

Q_CONSTEXPR std::array<QLatin1String, 4> STD_ENUMS =
{
    QLatin1String{"Err"},
    QLatin1String{"None"},
    QLatin1String{"Ok"},
    QLatin1String{"Some"}
};

Q_CONSTEXPR QLatin1String SHORT_OPERATORS{"=!?+-*/%&|^<>."};

Q_CONSTEXPR std::array<QLatin1String, 20> LONG_OPERATORS =
{
    QLatin1String{".."},
    QLatin1String{"<<"},
    QLatin1String{">>"},
    QLatin1String{"&&"},
    QLatin1String{"||"},
    QLatin1String{"=="},
    QLatin1String{"!="},
    QLatin1String{"<="},
    QLatin1String{">="},
    QLatin1String{"=>"},
    QLatin1String{"+="},
    QLatin1String{"-="},
    QLatin1String{"*="},
    QLatin1String{"/="},
    QLatin1String{"%="},
    QLatin1String{"&="},
    QLatin1String{"|="},
    QLatin1String{"^="},
    QLatin1String{"<<="},
    QLatin1String{">>="}
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
