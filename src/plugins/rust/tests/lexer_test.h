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

#include <QObject>

namespace Rust {
namespace Internal {

class LexerTest : public QObject
{
    Q_OBJECT

private slots:
    void empty();
    void state();
    void whitespace();
    void identifier();
    void keyword();
    void zero();
    void integer1();
    void integer2();
    void integer3();
    void integer4();
    void binary();
    void hexadecimal();
    void octal();
    void floating1();
    void floating2();
    void floating3();
    void floating4();
    void charLiteral1();
    void charLiteral2();
    void byte();
    void string();
    void byteString();
    void multiLineString1();
    void multiLineString2();
    void multiLineString3();
    void multiLineStringEscapedEol();
    void rawString1();
    void rawString2();
    void rawString3();
    void rawByteString1();
    void rawByteString2();
    void rawByteString3();
    void oneLineComment();
    void oneLineDocComment1();
    void oneLineDocComment2();
    void multiLineComment1();
    void multiLineComment2();
    void multiLineComment3();
    void multiLineDocComment1();
    void multiLineDocComment2();
    void braces();
};

} // namespace Internal
} // namespace Rust
