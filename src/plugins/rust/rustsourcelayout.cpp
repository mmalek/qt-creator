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

#include "rustsourcelayout.h"
#include "rusttoken.h"
#include <texteditor/textdocumentlayout.h>
#include <QTextBlock>

namespace Rust {
namespace Internal {
namespace SourceLayout {

namespace {

bool isIn(const QTextCursor &cursor, bool (*predicate)(TokenType))
{
    const int cursorPos = cursor.positionInBlock();
    const QTextBlock block = cursor.block();
    const QTextBlock previousBlock = block.previous();
    const QString text = block.text();

    Lexer lexer(&text,
                SourceLayout::multiLineState(previousBlock),
                SourceLayout::multiLineParam(previousBlock),
                SourceLayout::braceDepth(previousBlock));

    while(Token token = lexer.next()) {
        const int end = token.begin + token.length;
        if (cursorPos < end) {
            return predicate(token.type);
        }
    }

    return false;
}

constexpr bool isComment(const TokenType t)
{
    return t == TokenType::Comment || t == TokenType::DocComment;
}

constexpr bool isString(const TokenType t)
{
    return t == TokenType::String;
}

constexpr bool isCommentOrString(const TokenType t)
{
    return isComment(t) || isString(t);
}

} // namespace

Lexer::MultiLineState multiLineState(const QTextBlock &block)
{
    const int userState = block.userState();
    return static_cast<Lexer::MultiLineState>(qMax(userState, 0) & 0xFF);
}

quint8 multiLineParam(const QTextBlock &block)
{
    return TextEditor::TextDocumentLayout::lexerState(block);
}

quint8 braceDepth(const QTextBlock &block)
{
    const int userState = block.userState();
    return userState > 0 ? (userState >> 8) : 0;
}

void saveLexerState(QTextBlock &block, const Lexer& lexer)
{
    TextEditor::TextDocumentLayout::setLexerState(block, lexer.multiLineParam());
    block.setUserState((lexer.depth() << 8) | (static_cast<int>(lexer.multiLineState()) & 0xFF));
}

bool isInComment(const QTextCursor &cursor)
{
    return isIn(cursor, &isComment);
}

bool isInString(const QTextCursor &cursor)
{
    return isIn(cursor, &isString);
}

bool isInCommentOrString(const QTextCursor &cursor)
{
    return !isIn(cursor, &isCommentOrString);
}

} // namespace SourceLayout
} // namespace Internal
} // namespace Rust
