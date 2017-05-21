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

#include "rustplugin.h"
#include "rustmimetypes.h"
#include "rustprojectmanager.h"
#include "rustbuildconfiguration.h"
#include "rustbuildstep.h"
#include "rustrunconfiguration.h"
#include "rusteditorfactory.h"
#include "rusteditors.h"

#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/fileiconprovider.h>
#include <texteditor/texteditorconstants.h>
#include <utils/mimetypes/mimedatabase.h>

#include <QMenu>
#include <QtPlugin>

namespace Rust {
namespace Internal {

namespace {

const char MAIN_MENU[] = "Rust.Tools.Menu";

} // namespace

static Plugin *m_instance = 0;

Plugin::Plugin()
{
    m_instance = this;
}

Plugin::~Plugin()
{
    m_instance = 0;
}

bool Plugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorMessage)

    Utils::MimeDatabase::addMimeTypes(QLatin1String(":/Rust.mimetypes.xml"));

    addAutoReleasedObject(new ProjectManager);
    addAutoReleasedObject(new BuildConfigurationFactory);
    addAutoReleasedObject(new BuildStepFactory);
    addAutoReleasedObject(new RunConfigurationFactory);
    addAutoReleasedObject(new EditorFactory);

    // Add MIME overlay icons (these icons displayed at Project dock panel)
    const QIcon icon((QLatin1String(":/images/rust.svg")));
    if (!icon.isNull()) {
        Core::FileIconProvider::registerIconOverlayForMimeType(icon, MimeTypes::RUST_SOURCE);
        Core::FileIconProvider::registerIconOverlayForMimeType(icon, MimeTypes::CARGO_MANIFEST);
    }

    Core::Context context(Editors::RUST);

    Core::ActionContainer *mainMenu = Core::ActionManager::createMenu(MAIN_MENU);

    {
        QMenu *menu = mainMenu->menu();
        menu->setTitle(tr("&Rust"));
        menu->setEnabled(true);
    }

    Core::ActionManager::actionContainer(Core::Constants::M_TOOLS)->addMenu(mainMenu);

    Core::ActionContainer *contextMenu = Core::ActionManager::createMenu(Editors::CONTEXT_MENU);

    Core::Command *cmd;

    cmd = Core::ActionManager::command(TextEditor::Constants::FOLLOW_SYMBOL_UNDER_CURSOR);
    contextMenu->addAction(cmd);
    mainMenu->addAction(cmd);

    cmd = Core::ActionManager::command(TextEditor::Constants::FOLLOW_SYMBOL_UNDER_CURSOR_IN_NEXT_SPLIT);
    mainMenu->addAction(cmd);

    contextMenu->addSeparator(context);

    cmd = Core::ActionManager::command(TextEditor::Constants::AUTO_INDENT_SELECTION);
    contextMenu->addAction(cmd);

    cmd = Core::ActionManager::command(TextEditor::Constants::UN_COMMENT_SELECTION);
    contextMenu->addAction(cmd);

    return true;
}

void Plugin::extensionsInitialized()
{
}

} // namespace Internal
} // namespace Rust
