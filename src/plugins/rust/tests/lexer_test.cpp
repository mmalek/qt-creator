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

#include "lexer_test.h"

#include <rust/lexer.h>
#include <rust/token.h>

#include <QtTest>

namespace Rust {
namespace Internal {

namespace {

QByteArray toByteArray(TokenType tokenType)
{
    switch(tokenType)
    {
    case TokenType::Keyword: return "Keyword";
    case TokenType::Operator: return "Operator";
    case TokenType::Identifier: return "Identifier";
    case TokenType::String: return "String";
    case TokenType::Number: return "Number";
    case TokenType::Symbol: return "Symbol";
    case TokenType::Comment: return "Comment";
    case TokenType::None:
    default: return "None";
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

void LexerTest::empty()
{
    Lexer lexer{QStringRef()};
    QCOMPARE(lexer.state(), Lexer::State::Default);
    QCOMPARE(lexer.next().type, TokenType::None);
}

void LexerTest::state()
{
    Lexer lexer{QStringRef(), Lexer::State::String};
    QCOMPARE(lexer.state(), Lexer::State::String);
    QCOMPARE(lexer.next().type, TokenType::None);
}

void LexerTest::whitespace()
{
    const QString buffer{QLatin1String{"     "}};
    Lexer lexer{&buffer};
    QCOMPARE(lexer.state(), Lexer::State::Default);
    QCOMPARE(lexer.next().type, TokenType::None);
    QCOMPARE(lexer.state(), Lexer::State::Default);
}

void LexerTest::identifier()
{
    const QString buffer{QLatin1String{"   \t    abc123_890   "}};
    Lexer lexer{&buffer};
    QCOMPARE(lexer.state(), Lexer::State::Default);
    QCOMPARE(lexer.next(), (Token{8, 10, TokenType::Identifier}));
    QCOMPARE(lexer.state(), Lexer::State::Default);
    QCOMPARE(lexer.next().type, TokenType::None);
}

void LexerTest::keyword()
{
    const QString buffer{QLatin1String{"   \t    let   "}};
    Lexer lexer{&buffer};
    QCOMPARE(lexer.state(), Lexer::State::Default);
    QCOMPARE(lexer.next(), (Token{8, 3, TokenType::Keyword}));
    QCOMPARE(lexer.state(), Lexer::State::Default);
    QCOMPARE(lexer.next().type, TokenType::None);
}

void LexerTest::integer1()
{
    const QString buffer{QLatin1String{"   \t    678401   "}};
    Lexer lexer{&buffer};
    QCOMPARE(lexer.state(), Lexer::State::Default);
    QCOMPARE(lexer.next(), (Token{8, 6, TokenType::Number}));
    QCOMPARE(lexer.state(), Lexer::State::Default);
    QCOMPARE(lexer.next().type, TokenType::None);
}

void LexerTest::integer2()
{
    const QString buffer{QLatin1String{"   \t    678_401   "}};
    Lexer lexer{&buffer};
    QCOMPARE(lexer.state(), Lexer::State::Default);
    QCOMPARE(lexer.next(), (Token{8, 7, TokenType::Number}));
    QCOMPARE(lexer.state(), Lexer::State::Default);
    QCOMPARE(lexer.next().type, TokenType::None);
}

void LexerTest::integer3()
{
    const QString buffer{QLatin1String{"401i32"}};
    Lexer lexer{&buffer};
    QCOMPARE(lexer.state(), Lexer::State::Default);
    QCOMPARE(lexer.next(), (Token{0, 6, TokenType::Number}));
    QCOMPARE(lexer.state(), Lexer::State::Default);
    QCOMPARE(lexer.next().type, TokenType::None);
}

void LexerTest::integer4()
{
    const QString buffer{QLatin1String{"401u64"}};
    Lexer lexer{&buffer};
    QCOMPARE(lexer.state(), Lexer::State::Default);
    QCOMPARE(lexer.next(), (Token{0, 6, TokenType::Number}));
    QCOMPARE(lexer.state(), Lexer::State::Default);
    QCOMPARE(lexer.next().type, TokenType::None);
}

void LexerTest::floating1()
{
    const QString buffer{QLatin1String{"   \t    678401.   "}};
    Lexer lexer{&buffer};
    QCOMPARE(lexer.state(), Lexer::State::Default);
    QCOMPARE(lexer.next(), (Token{8, 7, TokenType::Number}));
    QCOMPARE(lexer.state(), Lexer::State::Default);
    QCOMPARE(lexer.next().type, TokenType::None);
}

void LexerTest::floating2()
{
    const QString buffer{QLatin1String{"   \t    678401.1  3_53.9  78."}};
    Lexer lexer{&buffer};
    QCOMPARE(lexer.state(), Lexer::State::Default);
    QCOMPARE(lexer.next(), (Token{8, 8, TokenType::Number}));
    QCOMPARE(lexer.state(), Lexer::State::Default);
    QCOMPARE(lexer.next(), (Token{18, 6, TokenType::Number}));
    QCOMPARE(lexer.state(), Lexer::State::Default);
    QCOMPARE(lexer.next(), (Token{26, 3, TokenType::Number}));
    QCOMPARE(lexer.state(), Lexer::State::Default);
    QCOMPARE(lexer.next().type, TokenType::None);
}

void LexerTest::floating3()
{
    const QString buffer{QLatin1String{"678401.2f32"}};
    Lexer lexer{&buffer};
    QCOMPARE(lexer.state(), Lexer::State::Default);
    QCOMPARE(lexer.next(), (Token{0, 11, TokenType::Number}));
    QCOMPARE(lexer.state(), Lexer::State::Default);
    QCOMPARE(lexer.next().type, TokenType::None);
}

} // namespace Internal
} // namespace Rust
