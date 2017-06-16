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
#include "rustplugin.h"
#include "rusttoolchainmanager.h"
#include "ui_rusttoolsoptionspage.h"

#include <projectexplorer/projectexplorerconstants.h>
#include <utils/treemodel.h>

namespace Rust {
namespace Internal {
namespace {

enum Column : int {
    ColumnName = 0,
    ColumnPath,
};

} // namespace

class ToolItemModel : public Utils::TreeModel<Utils::TreeItem, Utils::TreeItem, ToolItem>
{
    Q_DECLARE_TR_FUNCTIONS(Rust::ToolsOptionsPage)

public:
    ToolItemModel(QObject *parent, ToolChainManager& toolChainManager);

    Utils::TreeItem &autodetected() const { return *m_autodetected; }
    Utils::TreeItem &manual() const { return *m_manual; }

    Core::Id defaultItemId() const { return m_defaultItemId; }

    void apply();

private:
    ToolChainManager& m_toolChainManager;
    Utils::TreeItem *m_autodetected;
    Utils::TreeItem *m_manual;
    Core::Id m_defaultItemId;
};

class ToolItem : public Utils::TreeItem
{
    Q_DECLARE_TR_FUNCTIONS(Rust::ToolsOptionsPage)

public:
    ToolItem(ToolChain toolChain, bool changed = false) :
        m_toolChain(toolChain),
        m_changed(changed)
    {}

    ToolItemModel *model() const { return static_cast<ToolItemModel *>(TreeItem::model()); }

    QVariant data(int column, int role) const
    {
        switch (role) {
        case Qt::DisplayRole:
            switch (column) {
            case ColumnName:
                if (model()->defaultItemId() == m_toolChain.id)
                    return m_toolChain.name + tr(" (Default)");
                else
                    return m_toolChain.name;
            case ColumnPath:
                return m_toolChain.path.toUserOutput();

            default:
                Q_UNREACHABLE();
            }

        case Qt::FontRole: {
            QFont font;
            font.setBold(m_changed);
            return font;
        }
        }
        return QVariant();
    }

    ToolChain m_toolChain;
    bool m_changed = true;
};

ToolItemModel::ToolItemModel(QObject *parent, ToolChainManager& toolChainManager)
    : Utils::TreeModel<Utils::TreeItem, Utils::TreeItem, ToolItem>(parent),
      m_toolChainManager(toolChainManager),
      m_autodetected(new Utils::StaticTreeItem(tr("Auto-detected"))),
      m_manual(new Utils::StaticTreeItem(tr("Manual")))
{
    setHeader({tr("Name"), tr("Location")});
    rootItem()->appendChild(m_autodetected);
    rootItem()->appendChild(m_manual);

    for (const ToolChain& toolChain : m_toolChainManager.autodetected()) {
        autodetected().appendChild(new ToolItem(toolChain));
    }

    for (const ToolChain& toolChain : m_toolChainManager.manual()) {
        manual().appendChild(new ToolItem(toolChain));
    }
}

void ToolItemModel::apply()
{
    auto& toolChains = m_toolChainManager.manual();

    toolChains.clear();
    toolChains.reserve(manual().childCount());

    manual().forChildrenAtLevel(1, [&toolChains](const Utils::TreeItem* item) {
        toolChains.push_back(static_cast<const ToolItem*>(item)->m_toolChain);
    });
}

const char ToolsOptionsPage::ID[] = "Z.RustTools";

ToolsOptionsPage::ToolsOptionsPage(ToolChainManager& toolChainManager)
    : m_ui(new Ui::ToolsOptionsPage),
      m_model(new ToolItemModel(this, toolChainManager))
{
    setId(ID);
    setDisplayName(tr("Rust"));
    setCategory(ProjectExplorer::Constants::PROJECTEXPLORER_SETTINGS_CATEGORY);
    setDisplayCategory(QCoreApplication::translate("ProjectExplorer",
       ProjectExplorer::Constants::PROJECTEXPLORER_SETTINGS_TR_CATEGORY));
    setCategoryIcon(Utils::Icon(ProjectExplorer::Constants::PROJECTEXPLORER_SETTINGS_CATEGORY_ICON));
}

ToolsOptionsPage::~ToolsOptionsPage()
{
}

QWidget *ToolsOptionsPage::widget()
{
    if (!m_widget) {
        m_widget.reset(new QWidget);
        m_ui->setupUi(m_widget.data());
        m_ui->tools->setModel(m_model);
        m_ui->tools->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        m_ui->tools->header()->setSectionResizeMode(1, QHeaderView::Stretch);
        m_ui->tools->expandAll();
    }
    return m_widget.data();
}

void ToolsOptionsPage::apply()
{
    m_model->apply();
}

void ToolsOptionsPage::finish()
{
    m_widget.reset();
}

} // namespace Internal
} // namespace Rust
