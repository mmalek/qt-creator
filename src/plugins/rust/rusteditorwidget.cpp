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

#include "rusteditorwidget.h"
#include "rusteditors.h"
#include "rustracer.h"
#include "rustslice.h"
#include "rustsourcelayout.h"

#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <texteditor/textdocument.h>
#include <utils/fileutils.h>

#include <QMenu>

namespace Rust {
namespace Internal {

void EditorWidget::contextMenuEvent(QContextMenuEvent *e)
{
    QPointer<QMenu> menu(new QMenu(this));

    Core::ActionContainer *mcontext = Core::ActionManager::actionContainer(Editors::CONTEXT_MENU);
    QMenu *contextMenu = mcontext->menu();

    foreach (QAction *action, contextMenu->actions()) {
        menu->addAction(action);
    }

    appendStandardContextMenuActions(menu);

    menu->exec(e->globalPos());
    if (!menu)
        return;
    delete menu;
}

TextEditor::TextEditorWidget::Link EditorWidget::findLinkAt(const QTextCursor &textCursor,
                                                            bool resolveTarget,
                                                            bool inNextSplit)
{
    if (!SourceLayout::isInCommentOrString(textCursor)) {
        auto results = Racer::run(Racer::Request::FindDefinition,
                                  textCursor,
                                  textDocument()->filePath().toString());

        if (!results.isEmpty()) {
            const Racer::Result& result = results.first();
            Link link(result.filePath, result.line, result.column);
            if (const Slice slice = SourceLayout::identAtCursor(textCursor)) {
                link.linkTextStart = slice.begin;
                link.linkTextEnd = slice.begin + slice.length;
            }
            return link;
        }
    }

    return Link();
}

} // namespace Internal
} // namespace Rust
