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

#include "rusttoolsoptionspage.h"
#include "rustsettings.h"
#include "rusttoolchainmanager.h"
#include "ui_rusttoolsoptionspage.h"

namespace Rust {
namespace Internal {
namespace {

enum Column : int {
    ColumnName = 0,
    ColumnPath,
};

} // namespace

const char ToolsOptionsPage::ID[] = "Rust.ToolsOptions";

namespace {
const char CATEGORY_ID[] = "Z.Rust";
} // namespace

ToolsOptionsPage::ToolsOptionsPage(ToolChainManager& toolChainManager)
    : m_toolChainManager(toolChainManager),
      m_ui(new Ui::ToolsOptionsPage)
{
    setId(ID);
    setDisplayName(tr("Tools"));

    setCategory(CATEGORY_ID);
    setDisplayCategory(tr("Rust"));
    setCategoryIcon(QString(QLatin1String(":/images/rust.svg")));
}

ToolsOptionsPage::~ToolsOptionsPage()
{
}

QWidget *ToolsOptionsPage::widget()
{
    if (!m_widget) {
        m_widget.reset(new QWidget);
        m_ui->setupUi(m_widget.data());

        m_tools.push_back(Tool{&Settings::CARGO, m_ui->cargo});
        m_tools.push_back(Tool{&Settings::RUSTUP, m_ui->rustup});
        m_tools.push_back(Tool{&Settings::RACER, m_ui->racer});

        for (const Tool& tool : m_tools) {
            tool.widget->setExpectedKind(Utils::PathChooser::ExistingCommand);
            tool.widget->setEnvironment(m_toolChainManager.environment());
            tool.widget->setPath(Settings::value(*tool.option));
        }
    }
    return m_widget.data();
}

void ToolsOptionsPage::apply()
{
    bool changed = false;

    for (const Tool& tool : m_tools) {
        const QString path = tool.widget->rawPath();
        if (path != Settings::value(*tool.option)) {
            Settings::setValue(*tool.option, path);
            changed = true;
        }
    }

    if (changed) {
        m_toolChainManager.settingsChanged();
    }
}

void ToolsOptionsPage::finish()
{
    m_tools.clear();
    m_widget.reset();
}

} // namespace Internal
} // namespace Rust
