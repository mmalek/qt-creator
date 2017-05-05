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

#include "syntaxhighlighter.h"
#include "lexer.h"
#include "token.h"

namespace Rust {
namespace Internal {

SyntaxHighlighter::SyntaxHighlighter()
{
    setTextFormatCategories({
                                TextEditor::C_LABEL,
                                TextEditor::C_WARNING,
                                TextEditor::C_KEYWORD,
                                TextEditor::C_OPERATOR,
                                TextEditor::C_LABEL,
                                TextEditor::C_STRING,
                                TextEditor::C_STRING,
                                TextEditor::C_NUMBER,
                                TextEditor::C_COMMENT,
                                TextEditor::C_PRIMITIVE_TYPE,
                                TextEditor::C_TYPE
                            });
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    Lexer lexer(&text, previousBlockState());

    while (true)
    {
        const Token token = lexer.next();
        if (token.type == TokenType::None) {
            break;
        } else if (token.type != TokenType::Identifier) {
            setFormat(token.begin, token.length, formatForCategory(static_cast<int>(token.type)));
        }
    }

    setCurrentBlockState(static_cast<int>(lexer));
}

} // namespace Internal
} // namespace Rust
