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

#include <texteditor/textdocumentlayout.h>

namespace Rust {
namespace Internal {
namespace {
enum class Category {
    NoFormatting = -1,
    Keyword = 0,
    Operator,
    String,
    Number,
    Comment,
    DocComment,
    PrimitiveType,
    Type,
    Attribute,
    Whitespace,
    Error,

    NumCategories
};

Category toCategory(TokenType tokenType)
{
    static_assert(static_cast<int>(Category::NumCategories) == 11,
                  "Number of categories changed, update the code below");

    switch (tokenType) {
    case TokenType::Keyword:
        return Category::Keyword;
    case TokenType::Operator:
        return Category::Operator;
    case TokenType::Char:
    case TokenType::String:
        return Category::String;
    case TokenType::Number:
        return Category::Number;
    case TokenType::Comment:
        return Category::Comment;
    case TokenType::DocComment:
        return Category::DocComment;
    case TokenType::PrimitiveType:
        return Category::PrimitiveType;
    case TokenType::Type:
        return Category::Type;
    case TokenType::Attribute:
        return Category::Attribute;
    case TokenType::Unknown:
        return Category::Error;
    default:
        return Category::NoFormatting;
    }
}

} // namespace

SyntaxHighlighter::SyntaxHighlighter()
{
    static_assert(static_cast<int>(Category::NumCategories) == 11,
                  "Number of categories changed, update the code below");

    static QVector<TextEditor::TextStyle> categories{
                TextEditor::C_KEYWORD,
                TextEditor::C_OPERATOR,
                TextEditor::C_STRING,
                TextEditor::C_NUMBER,
                TextEditor::C_COMMENT,
                TextEditor::C_DOXYGEN_COMMENT,
                TextEditor::C_PRIMITIVE_TYPE,
                TextEditor::C_TYPE,
                TextEditor::C_PREPROCESSOR,
                TextEditor::C_VISUAL_WHITESPACE,
                TextEditor::C_ERROR
    };

    setTextFormatCategories(categories);
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    Lexer lexer(&text, previousBlockState());

    TextEditor::Parentheses parentheses;

    while (const Token token = lexer.next())
    {
        const Category category = toCategory(token.type);
        if (category != Category::NoFormatting) {
            setFormat(token.begin, token.length, formatForCategory(static_cast<int>(category)));
        }

        if (token.type == TokenType::ParenthesisLeft ||
            token.type == TokenType::BraceLeft ||
            token.type == TokenType::SquareBracketLeft) {
            parentheses.push_back(TextEditor::Parenthesis(TextEditor::Parenthesis::Opened,
                                                          text[token.begin], token.begin));
        } else if (token.type == TokenType::ParenthesisRight ||
                   token.type == TokenType::BraceRight ||
                   token.type == TokenType::SquareBracketRight) {
           parentheses.push_back(TextEditor::Parenthesis(TextEditor::Parenthesis::Closed,
                                                         text[token.begin], token.begin));
        }
    }

    setCurrentBlockState(static_cast<int>(lexer));

    TextEditor::TextDocumentLayout::setParentheses(currentBlock(), parentheses);

    highlightWhitespace(text);
}

void SyntaxHighlighter::highlightWhitespace(const QString &text)
{
    int beginWhitespace = -1;

    for (int i = 0; i < text.size(); ++i) {
        const bool isWhitespace = text[i].isSpace();
        if (isWhitespace && beginWhitespace < 0) {
            beginWhitespace = i;
        } else if (!isWhitespace && beginWhitespace >= 0) {
            setFormat(beginWhitespace, i - beginWhitespace,
                      formatForCategory(static_cast<int>(Category::Whitespace)));
            beginWhitespace = -1;
        }
    }

    if (beginWhitespace >= 0 && beginWhitespace < text.size()) {
        setFormat(beginWhitespace, text.size() - beginWhitespace,
                  formatForCategory(static_cast<int>(Category::Whitespace)));
    }
}

} // namespace Internal
} // namespace Rust
