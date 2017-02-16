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

#include "rseditorfactory.h"
#include "editors.h"
#include "racercompletionassist.h"
#include "mimetypes.h"

#include <texteditor/basehoverhandler.h>
#include <texteditor/normalindenter.h>
#include <texteditor/textdocument.h>
#include <texteditor/texteditoractionhandler.h>
#include <utils/uncommentselection.h>

namespace Rust {
namespace Internal {

using namespace TextEditor;

RsEditorFactory::RsEditorFactory()
{
    setId(Editors::RUST);
    setDisplayName(tr("Rust Editor"));
    addMimeType(MimeTypes::RUST_SOURCE);
    addHoverHandler(new BaseHoverHandler);

    setEditorActionHandlers(TextEditorActionHandler::Format
                            | TextEditorActionHandler::UnCommentSelection
                            | TextEditorActionHandler::UnCollapseAll);

    setCommentStyle(Utils::CommentDefinition::CppStyle);
    setCompletionAssistProvider(new RacerCompletionAssistProvider);
    setDocumentCreator([]{
        auto document = new TextDocument(Editors::RUST);
        document->setMimeType(QLatin1String(MimeTypes::RUST_SOURCE));
        return document;
    });

    setIndenterCreator([]() { return new NormalIndenter; });
    setUseGenericHighlighter(true);
}

} // namespace Internal
} // namespace Rust
