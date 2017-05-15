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

#include <QTextBlock>
#include <QTextCursor>

namespace Rust {
namespace Internal {

bool AutoCompleter::contextAllowsAutoBrackets(const QTextCursor &cursor, const QString &textToInsert) const
{
    Q_UNUSED(textToInsert);
    return !SourceLayout::isInCommentOrString(cursor);
}

bool AutoCompleter::contextAllowsAutoQuotes(const QTextCursor &cursor, const QString &textToInsert) const
{
    Q_UNUSED(textToInsert);
    return !SourceLayout::isInCommentOrString(cursor);
}

bool AutoCompleter::contextAllowsElectricCharacters(const QTextCursor &cursor) const
{
    return !SourceLayout::isInCommentOrString(cursor);
}

bool AutoCompleter::isInComment(const QTextCursor &cursor) const
{
    return !SourceLayout::isInComment(cursor);
}

bool AutoCompleter::isInString(const QTextCursor &cursor) const
{
    return !SourceLayout::isInString(cursor);
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
