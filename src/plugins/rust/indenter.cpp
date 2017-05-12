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
#include <texteditor/textdocumentlayout.h>

namespace Rust {
namespace Internal {
namespace {

bool hasCloseBraceAtTheBeginning(const QTextBlock& block)
{
    const QString text = block.text();
    const int pos = TextEditor::TabSettings::firstNonSpace(text);
    return pos < text.size() && text[pos] == QLatin1Char('}');
}

} // namespace

bool Indenter::isElectricCharacter(const QChar &ch) const
{
    return ch == QLatin1Char('}');
}

void Indenter::indentBlock(QTextDocument *doc,
                           const QTextBlock &block,
                           const QChar &typedChar,
                           const TextEditor::TabSettings &tabSettings)
{
    Q_UNUSED(doc);
    Q_UNUSED(typedChar);

    int indent = indentFor(block, tabSettings);

    if (indent >= 0) {
        tabSettings.indentLine(block, indent);
    }
}

int Indenter::indentFor(const QTextBlock &block, const TextEditor::TabSettings &tabSettings)
{
    QTextBlock previousBlock = block.previous();
    if (previousBlock.isValid()) {
        int depth = TextEditor::TextDocumentLayout::braceDepth(previousBlock);
        if (hasCloseBraceAtTheBeginning(block)) {
            --depth;
        }

        QTextBlock prevPrevBlock = previousBlock.previous();
        const int previousDepth = TextEditor::TextDocumentLayout::braceDepth(prevPrevBlock);
        const QString previousText = previousBlock.text();
        Lexer prevLexer(&previousText,
                        SourceLayout::multiLineState(prevPrevBlock),
                        SourceLayout::multiLineParam(prevPrevBlock),
                        SourceLayout::braceDepth(prevPrevBlock));

        bool letWithoutEnd = false;

        while (const Token token = prevLexer.next()) {
            if (token.type == TokenType::Keyword) {
                const QStringRef text = previousText.midRef(token.begin, token.length);
                if (text == QLatin1String("let") || text == QLatin1String("const")) {
                    letWithoutEnd = true;
                }
            } else if (token.type == TokenType::Semicolon) {
                letWithoutEnd = false;
            }
        }

        if (depth <= previousDepth && letWithoutEnd) {
            ++depth;
        }

        const int adjust = depth * tabSettings.m_indentSize;
        return qMax(0, adjust);
    } else {
        return 0;
    }
}

} // namespace Internal
} // namespace Rust
