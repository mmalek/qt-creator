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
#include "rusttoken.h"

#include <QtTest>

using namespace Rust::Internal;

namespace {

QByteArray toByteArray(TokenType tokenType)
{
    static_assert(static_cast<int>(TokenType::NumTokenTypes) == 23,
                  "Number of tokens changed, update the code below");

    switch(tokenType)
    {
    case TokenType::Keyword: return "Keyword";
    case TokenType::Operator: return "Operator";
    case TokenType::Identifier: return "Identifier";
    case TokenType::Char: return "Char";
    case TokenType::String: return "String";
    case TokenType::Number: return "Number";
    case TokenType::Comment: return "Comment";
    case TokenType::DocComment: return "DocComment";
    case TokenType::PrimitiveType: return "PrimitiveType";
    case TokenType::Type: return "Type";
    case TokenType::Enumeration: return "Enumeration";
    case TokenType::Comma: return "Comma";
    case TokenType::Colon: return "Colon";
    case TokenType::Semicolon: return "Semicolon";
    case TokenType::ParenthesisLeft: return "ParenthesisLeft";
    case TokenType::ParenthesisRight: return "ParenthesisRight";
    case TokenType::SquareBracketLeft: return "SquareBracketLeft";
    case TokenType::SquareBracketRight: return "SquareBracketRight";
    case TokenType::BraceLeft: return "BraceLeft";
    case TokenType::BraceRight: return "BraceRight";
    case TokenType::Attribute: return "Attribute";
    case TokenType::None: return "None";
    case TokenType::Unknown:
    default: return "Unknown";
    }
}

} // namespace

char *toString(TokenType tokenType)
{
    // bring QTest::toString overloads into scope:
    using QTest::toString;
    // delegate char* handling to QTest::toString(QByteArray):
    return toString(toByteArray(tokenType));
}

char *toString(const Token &token)
{
    // bring QTest::toString overloads into scope:
    using QTest::toString;
    // delegate char* handling to QTest::toString(QByteArray):
    return toString("Token{" +
                    QByteArray::number(token.begin) + ", " +
                    QByteArray::number(token.length) + ", " +
                    toByteArray(token.type) + '}');
}

class tst_RustLexer : public QObject
{
    Q_OBJECT

private slots:

    void empty()
    {
        Lexer lexer{QStringRef()};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void state()
    {
        Lexer lexer{QStringRef(), Lexer::MultiLineState::String, 0};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::String);
        QCOMPARE(lexer.multiLineParam(), (quint8)0);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void whitespace()
    {
        const QString buffer{QLatin1String{"     "}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
    }

    void identifier()
    {
        const QString buffer{QLatin1String{"   \t    abc123_890   "}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{8, 10, TokenType::Identifier}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void keyword()
    {
        const QString buffer{QLatin1String{"   \t    let   "}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{8, 3, TokenType::Keyword}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void zero()
    {
        const QString buffer{QLatin1String{"0  0i32  0E+3f32"}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 1, TokenType::Number}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{3, 4, TokenType::Number}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{9, 7, TokenType::Number}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void integer1()
    {
        const QString buffer{QLatin1String{"   \t    678401   "}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{8, 6, TokenType::Number}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void integer2()
    {
        const QString buffer{QLatin1String{"   \t    678_401   "}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{8, 7, TokenType::Number}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void integer3()
    {
        const QString buffer{QLatin1String{"401i32"}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 6, TokenType::Number}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void integer4()
    {
        const QString buffer{QLatin1String{"401u64 0589B34"}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 6, TokenType::Number}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{7, 7, TokenType::Unknown}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void binary()
    {
        const QString buffer{QLatin1String{"0b0111010 0b012"}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 9, TokenType::Number}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{10, 5, TokenType::Unknown}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void hexadecimal()
    {
        const QString buffer{QLatin1String{"0x0123456789AbCdEf 0x34G"}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 18, TokenType::Number}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{19, 5, TokenType::Unknown}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void octal()
    {
        const QString buffer{QLatin1String{"0o01234567 0o058"}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 10, TokenType::Number}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{11, 5, TokenType::Unknown}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void floating1()
    {
        const QString buffer{QLatin1String{"   \t    678401.   "}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{8, 7, TokenType::Number}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void floating2()
    {
        const QString buffer{QLatin1String{"   \t    678401.1  3_53.9  78."}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{8, 8, TokenType::Number}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{18, 6, TokenType::Number}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{26, 3, TokenType::Number}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void floating3()
    {
        const QString buffer{QLatin1String{"678401.2f32 1.3f128"}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 11, TokenType::Number}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{12, 7, TokenType::Unknown}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void floating4()
    {
        const QString buffer{QLatin1String{"678401.3E+4 5e+10 6e-1f64 6e+1i64"}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 11, TokenType::Number}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{12, 5, TokenType::Number}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{18, 7, TokenType::Number}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{26, 7, TokenType::Unknown}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void charLiteral1()
    {
        const QString buffer{QLatin1String{" 'b' '\\x7D' '\\u{3DF}' '\\n' '\\r' '\\t' '\\0' '\\\\' '\\\'' '\\\"'"}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{1, 3, TokenType::Char}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{5, 6, TokenType::Char}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{12, 9, TokenType::Char}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{22, 4, TokenType::Char}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{27, 4, TokenType::Char}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{32, 4, TokenType::Char}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{37, 4, TokenType::Char}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{42, 4, TokenType::Char}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{47, 4, TokenType::Char}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{52, 4, TokenType::Char}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void charLiteral2()
    {
        const QString buffer{QLatin1String{" '\"' ''' "}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{1, 3, TokenType::Char}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void byte()
    {
        const QString buffer{QLatin1String{" b'b' b'\\x7D'"}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{1, 4, TokenType::Char}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{6, 7, TokenType::Char}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void string()
    {
        const QString buffer{QLatin1String{"\"abc\" \"unicode char 4A5: \\u{4A5}\""}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 5, TokenType::String}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{6, 27, TokenType::String}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void byteString()
    {
        const QString buffer{QLatin1String{"b\"abc\""}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 6, TokenType::String}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void multiLineString1()
    {
        const QString buffer{QLatin1String{"\"abc\ndef\""}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 5, TokenType::String}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::String);
        QCOMPARE(lexer.multiLineParam(), (quint8)0);
        QCOMPARE(lexer.next(), (Token{5, 4, TokenType::String}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void multiLineString2()
    {
        const QString buffer{QLatin1String{"\"abc"}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 4, TokenType::String}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::String);
        QCOMPARE(lexer.multiLineParam(), (quint8)0);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void multiLineString3()
    {
        const QString buffer{QLatin1String{"def\""}};
        Lexer lexer{&buffer, Lexer::MultiLineState::String};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::String);
        QCOMPARE(lexer.multiLineParam(), (quint8)0);
        QCOMPARE(lexer.next(), (Token{0, 4, TokenType::String}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void multiLineStringEscapedEol()
    {
        const QString buffer{QLatin1String{"\"abc\\\ndef\""}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 6, TokenType::String}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::String);
        QCOMPARE(lexer.multiLineParam(), (quint8)0);
        QCOMPARE(lexer.next(), (Token{6, 4, TokenType::String}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void rawString1()
    {
        const QString buffer{QLatin1String{"r#\"abc\"#  r##\"def\nghi\"### r#\"ghi\" \"# "}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 8, TokenType::String}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{10, 8, TokenType::String}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::String);
        QCOMPARE(lexer.multiLineParam(), (quint8)2);
        QCOMPARE(lexer.next(), (Token{18, 6, TokenType::String}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{24, 1, TokenType::Unknown}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{26, 10, TokenType::String}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void rawString2()
    {
        const QString buffer{QLatin1String{"r#\"abc"}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 6, TokenType::String}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::String);
        QCOMPARE(lexer.multiLineParam(), (quint8)1);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void rawString3()
    {
        const QString buffer{QLatin1String{"def\"#"}};
        Lexer lexer{&buffer, Lexer::MultiLineState::String, 1};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::String);
        QCOMPARE(lexer.multiLineParam(), (quint8)1);
        QCOMPARE(lexer.next(), (Token{0, 5, TokenType::String}));
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void rawByteString1()
    {
        const QString buffer{QLatin1String{"br#\"abc\"#  br##\"def\nghi\"### br#\"ghi\" \"# "}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 9, TokenType::String}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{11, 9, TokenType::String}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::String);
        QCOMPARE(lexer.multiLineParam(), (quint8)2);
        QCOMPARE(lexer.next(), (Token{20, 6, TokenType::String}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{26, 1, TokenType::Unknown}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{28, 11, TokenType::String}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void rawByteString2()
    {
        const QString buffer{QLatin1String{"br#\"abc"}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 7, TokenType::String}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::String);
        QCOMPARE(lexer.multiLineParam(), (quint8)1);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void rawByteString3()
    {
        const QString buffer{QLatin1String{"def\"#"}};
        Lexer lexer{&buffer, Lexer::MultiLineState::String, 1};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::String);
        QCOMPARE(lexer.multiLineParam(), (quint8)1);
        QCOMPARE(lexer.next(), (Token{0, 5, TokenType::String}));
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void oneLineComment()
    {
        const QString buffer{QLatin1String{"// abc\n\rdef"}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 8, TokenType::Comment}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{8, 3, TokenType::Identifier}));
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void oneLineDocComment1()
    {
        const QString buffer{QLatin1String{"/// abc"}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 7, TokenType::DocComment}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void oneLineDocComment2()
    {
        const QString buffer{QLatin1String{"//! abc"}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 7, TokenType::DocComment}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void multiLineComment1()
    {
        const QString buffer{QLatin1String{"/* abc\n\rdef*/ghi"}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 8, TokenType::Comment}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Comment);
        QCOMPARE(lexer.multiLineParam(), (quint8)1);
        QCOMPARE(lexer.next(), (Token{8, 5, TokenType::Comment}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{13, 3, TokenType::Identifier}));
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void multiLineComment2()
    {
        const QString buffer{QLatin1String{" abc/*def"}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{1, 3, TokenType::Identifier}));
        QCOMPARE(lexer.next(), (Token{4, 5, TokenType::Comment}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Comment);
        QCOMPARE(lexer.multiLineParam(), (quint8)1);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void multiLineComment3()
    {
        const QString buffer{QLatin1String{" abc*/def"}};
        Lexer lexer{&buffer, Lexer::MultiLineState::Comment, 1};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Comment);
        QCOMPARE(lexer.multiLineParam(), (quint8)1);
        QCOMPARE(lexer.next(), (Token{0, 6, TokenType::Comment}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{6, 3, TokenType::Identifier}));
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void multiLineDocComment1()
    {
        const QString buffer{QLatin1String{"/** abc */ "}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 10, TokenType::DocComment}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void multiLineDocComment2()
    {
        const QString buffer{QLatin1String{"/*! abc */ "}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 10, TokenType::DocComment}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void parentheses()
    {
        const QString buffer{QLatin1String{"fn foo(bar: i64);"}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 2, TokenType::Keyword}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{3, 3, TokenType::Identifier}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{6, 1, TokenType::ParenthesisLeft}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{7, 3, TokenType::Identifier}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{10, 1, TokenType::Colon}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{12, 3, TokenType::PrimitiveType}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{15, 1, TokenType::ParenthesisRight}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{16, 1, TokenType::Semicolon}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void squareBrackets()
    {
        const QString buffer{QLatin1String{"let v = [1, 2];"}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 3, TokenType::Keyword}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{4, 1, TokenType::Identifier}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{6, 1, TokenType::Operator}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{8, 1, TokenType::SquareBracketLeft}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{9, 1, TokenType::Number}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{10, 1, TokenType::Comma}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{12, 1, TokenType::Number}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{13, 1, TokenType::SquareBracketRight}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{14, 1, TokenType::Semicolon}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void braces()
    {
        const QString buffer{QLatin1String{"struct My{ a: i32; }"}};
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 6, TokenType::Keyword}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{7, 2, TokenType::Identifier}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{9, 1, TokenType::BraceLeft}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{11, 1, TokenType::Identifier}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{12, 1, TokenType::Colon}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{14, 3, TokenType::PrimitiveType}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{17, 1, TokenType::Semicolon}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{19, 1, TokenType::BraceRight}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void attribute1()
    {
        const QString buffer = QStringLiteral("#[macro_use]");
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 12, TokenType::Attribute}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void attribute2()
    {
        const QString buffer = QStringLiteral("#[derive(Clone)]");
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 16, TokenType::Attribute}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }

    void attribute3()
    {
        const QString buffer = QStringLiteral("#![allow(non_snake_case)]");
        Lexer lexer{&buffer};
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next(), (Token{0, 25, TokenType::Attribute}));
        QCOMPARE(lexer.multiLineState(), Lexer::MultiLineState::Default);
        QCOMPARE(lexer.next().type, TokenType::None);
    }
};

QTEST_MAIN(tst_RustLexer)

#include "tst_rustlexer.moc"
