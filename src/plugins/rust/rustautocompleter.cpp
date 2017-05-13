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

#include "rustautocompleter.h"
#include "rustsourcelayout.h"
#include "rustlexer.h"
#include "rusttoken.h"

#include <QTextBlock>
#include <QTextCursor>

namespace Rust {
namespace Internal {

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

bool AutoCompleter::contextAllowsAutoBrackets(const QTextCursor &cursor, const QString &textToInsert) const
{
    Q_UNUSED(textToInsert);
    return !isIn(cursor, &isCommentOrString);
}

bool AutoCompleter::contextAllowsAutoQuotes(const QTextCursor &cursor, const QString &textToInsert) const
{
    Q_UNUSED(textToInsert);
    return !isIn(cursor, &isCommentOrString);
}

bool AutoCompleter::contextAllowsElectricCharacters(const QTextCursor &cursor) const
{
    return !isIn(cursor, &isCommentOrString);
}

bool AutoCompleter::isInComment(const QTextCursor &cursor) const
{
    return isIn(cursor, &isComment);
}

bool AutoCompleter::isInString(const QTextCursor &cursor) const
{
    return isIn(cursor, &isString);
}

QString AutoCompleter::insertMatchingBrace(const QTextCursor &cursor, const QString &text, QChar lookAhead, bool skipChars, int *skippedChars) const
{
    return TextEditor::AutoCompleter::insertMatchingBrace(cursor, text, lookAhead, skipChars, skippedChars);
}

QString AutoCompleter::insertMatchingQuote(const QTextCursor &cursor, const QString &text, QChar lookAhead, bool skipChars, int *skippedChars) const
{
    return TextEditor::AutoCompleter::insertMatchingQuote(cursor, text, lookAhead, skipChars, skippedChars);
}

QString AutoCompleter::insertParagraphSeparator(const QTextCursor &cursor) const
{
    return TextEditor::AutoCompleter::insertParagraphSeparator(cursor);
}

} // namespace Internal
} // namespace Rust
