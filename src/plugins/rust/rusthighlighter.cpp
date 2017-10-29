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

#include "rusthighlighter.h"
#include "rustgrammar.h"
#include "rustlexer.h"
#include "rusttoken.h"
#include "rustsourcelayout.h"

#include <texteditor/textdocumentlayout.h>

namespace Rust {
namespace Internal {
namespace {
enum Category {
    CategoryNoFormatting = -1,
    CategoryKeyword = 0,
    CategoryOperator,
    CategoryString,
    CategoryNumber,
    CategoryComment,
    CategoryDocComment,
    CategoryPrimitiveType,
    CategoryType,
    CategoryEnumeration,
    CategoryAttribute,
    CategoryWhitespace,
    CategoryError,

    NumCategories
};

Category toCategory(TokenType tokenType)
{
    static_assert(NumCategories == 12, "Number of categories changed, update the code below");

    switch (tokenType) {
    case TokenType::Keyword:
        return CategoryKeyword;
    case TokenType::Comma:
    case TokenType::Colon:
    case TokenType::Semicolon:
    case TokenType::Operator:
        return CategoryOperator;
    case TokenType::Char:
    case TokenType::String:
        return CategoryString;
    case TokenType::Number:
        return CategoryNumber;
    case TokenType::Comment:
        return CategoryComment;
    case TokenType::DocComment:
        return CategoryDocComment;
    case TokenType::PrimitiveType:
        return CategoryPrimitiveType;
    case TokenType::Type:
        return CategoryType;
    case TokenType::Enumeration:
        return CategoryEnumeration;
    case TokenType::Attribute:
        return CategoryAttribute;
    case TokenType::Unknown:
        return CategoryError;
    default:
        return CategoryNoFormatting;
    }
}

inline void setFoldingEndIncluded(const QTextBlock& block, bool included)
{
    if (TextEditor::TextBlockUserData* data = TextEditor::TextDocumentLayout::userData(block)) {
        data->setFoldingEndIncluded(included);
    }
}

static TextEditor::TextStyle styleForFormat(int format)
{
    static_assert(NumCategories == 12, "Number of categories changed, update the code below");

    switch (format) {
    case CategoryKeyword: return TextEditor::C_KEYWORD;
    case CategoryOperator: return TextEditor::C_OPERATOR;
    case CategoryString: return TextEditor::C_STRING;
    case CategoryNumber: return TextEditor::C_NUMBER;
    case CategoryComment: return TextEditor::C_COMMENT;
    case CategoryDocComment: return TextEditor::C_DOXYGEN_COMMENT;
    case CategoryPrimitiveType: return TextEditor::C_PRIMITIVE_TYPE;
    case CategoryType: return TextEditor::C_TYPE;
    case CategoryEnumeration: return TextEditor::C_ENUMERATION;
    case CategoryAttribute: return TextEditor::C_PREPROCESSOR;
    case CategoryWhitespace: return TextEditor::C_VISUAL_WHITESPACE;
    case CategoryError: return TextEditor::C_ERROR;
    default: return TextEditor::C_TEXT;
    };

}

} // namespace

Highlighter::Highlighter()
{
    setTextFormatCategories(NumCategories, styleForFormat);
}

void Highlighter::highlightBlock(const QString &text)
{
    QTextBlock block = currentBlock();
    QTextBlock previousBlock = block.previous();

    int currentDepth = SourceLayout::braceDepth(previousBlock);

    Lexer lexer(&text,
                SourceLayout::multiLineState(previousBlock),
                SourceLayout::multiLineParam(previousBlock),
                currentDepth);

    TextEditor::Parentheses parentheses;

    int tokenCount = 0;
    bool hasFirstRightBrace = false;
    bool hasSecondElse = false;

    while (const Token token = lexer.next())
    {
        const Category category = toCategory(token.type);
        if (category != CategoryNoFormatting) {
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

        if (tokenCount == 0) {
            hasFirstRightBrace = (token.type == TokenType::BraceRight);
        } else if (tokenCount == 1) {
            hasSecondElse = (token.type == TokenType::Keyword &&
                             text.midRef(token.begin, token.length) == KEYWORD_ELSE);
        }

        ++tokenCount;
    }

    formatSpaces(text, 0, text.size());

    if (hasFirstRightBrace && hasSecondElse) {
        if (currentDepth > 0) {
            --currentDepth;
        }

        setFoldingEndIncluded(previousBlock, false);
    }

    SourceLayout::saveLexerState(block, lexer);

    TextEditor::TextDocumentLayout::setParentheses(block, parentheses);
    TextEditor::TextDocumentLayout::setFoldingIndent(block, currentDepth);

    setFoldingEndIncluded(block, true);
}

} // namespace Internal
} // namespace Rust
