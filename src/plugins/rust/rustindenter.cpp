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

#include "rustindenter.h"
#include "rustgrammar.h"
#include "rustsourcelayout.h"
#include <texteditor/tabsettings.h>
#include <texteditor/textdocumentlayout.h>

namespace Rust {
namespace Internal {

bool Indenter::isElectricCharacter(const QChar &ch) const
{
    return ch == CHAR_BRACE_RIGHT;
}

int Indenter::indentFor(const QTextBlock &block, const TextEditor::TabSettings &tabSettings)
{
    QTextBlock previousBlock = block.previous();
    if (previousBlock.isValid()) {
        int depth = TextEditor::TextDocumentLayout::braceDepth(previousBlock);
        if (SourceLayout::hasCloseBraceAtTheBeginning(block)) {
            --depth;
        }

        QTextBlock prevPrevBlock = previousBlock.previous();
        const int previousDepth = TextEditor::TextDocumentLayout::braceDepth(prevPrevBlock);

        if (depth != previousDepth) {
            const int indent = depth * tabSettings.m_indentSize;
            return qMax(0, indent);
        } else {
            return tabSettings.indentationColumn(previousBlock.text());
        }
    } else {
        return 0;
    }
}

} // namespace Internal
} // namespace Rust
