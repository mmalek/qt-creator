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

#include "plugin.h"
#include "mimetypes.h"
#include "projectmanager.h"
#include "buildconfigurationfactory.h"
#include "buildstepfactory.h"

#include <coreplugin/fileiconprovider.h>
#include <utils/mimetypes/mimedatabase.h>

#include <QtPlugin>

namespace Rust {

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

    // Add MIME overlay icons (these icons displayed at Project dock panel)
    const QIcon icon((QLatin1String(":/images/rust.svg")));
    if (!icon.isNull()) {
        Core::FileIconProvider::registerIconOverlayForMimeType(icon, MimeTypes::RUST_SOURCE);
        Core::FileIconProvider::registerIconOverlayForMimeType(icon, MimeTypes::CARGO_MANIFEST);
    }

    return true;
}

void Plugin::extensionsInitialized()
{
}

} // namespace Rust
