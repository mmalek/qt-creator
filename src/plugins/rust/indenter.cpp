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

#include "indenter.h"
#include "lexer.h"
#include "token.h"
#include "rustsourcelayout.h"
#include <texteditor/tabsettings.h>

namespace Rust {
namespace Internal {

constexpr bool isOpenBracket(const QChar c)
{
    return c == QLatin1Char('{') || c == QLatin1Char('(') || c == QLatin1Char('[');
}

constexpr bool isCloseBracket(const QChar c)
{
    return c == QLatin1Char('}') || c == QLatin1Char(')') || c == QLatin1Char(']');
}

bool Indenter::isElectricCharacter(const QChar &ch) const
{
    return isOpenBracket(ch) || isCloseBracket(ch) || ch == QLatin1Char(';');
}

void Indenter::indentBlock(QTextDocument *doc,
                           const QTextBlock &block,
                           const QChar &typedChar,
                           const TextEditor::TabSettings &tabSettings)
{
    Q_UNUSED(doc)
    Q_UNUSED(typedChar)

    int indent = indentFor(block, tabSettings);

    if (indent > 0) {
        tabSettings.indentLine(block, indent);
    }
}

int Indenter::indentFor(const QTextBlock &block, const TextEditor::TabSettings &tabSettings)
{
    QTextBlock previousBlock = block.previous();
    if (previousBlock.isValid()) {
        const QString previousText = previousBlock.text();

        const int indent = tabSettings.indentationColumn(previousText);

        Lexer prevLexer(&previousText,
                        SourceLayout::multiLineState(previousBlock),
                        SourceLayout::multiLineParam(previousBlock),
                        SourceLayout::braceDepth(previousBlock));

        bool letWithoutEnd = false;

        int delta = 0;
        while (const Token token = prevLexer.next()) {
            if (token.type == TokenType::Keyword) {
                if (previousText.midRef(token.begin, token.length) == QLatin1String("let")) {
                    letWithoutEnd = true;
                }
            } else if (token.type == TokenType::Semicolon) {
                letWithoutEnd = false;
            } else if (token.type == TokenType::BraceLeft ||
                token.type == TokenType::SquareBracketLeft ||
                token.type == TokenType::ParenthesisLeft) {
                ++delta;
            } else if (token.type == TokenType::BraceRight ||
                       token.type == TokenType::SquareBracketRight ||
                       token.type == TokenType::ParenthesisRight) {
                --delta;
            }
        }

        if (delta <= 0 && letWithoutEnd) {
            delta = 1;
        }

        delta -= [&block] {
            const QString text = block.text();
            Lexer lexer(&text,
                        SourceLayout::multiLineState(block  ),
                        SourceLayout::multiLineParam(block),
                        SourceLayout::braceDepth(block));
            const Token token = lexer.next();
            return (token && token.type == TokenType::BraceRight) ? 1 : 0;
        } ();

        const int adjust = delta * tabSettings.m_indentSize;
        return qMax(0, indent + adjust);
    } else {
        return 0;
    }
}

} // namespace Internal
} // namespace Rust
