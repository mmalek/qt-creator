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

#include "rustkitconfigwidget.h"
#include "rustkitinformation.h"
#include "rusttoolchainmanager.h"
#include "rusttoolsoptionspage.h"

#include <coreplugin/icore.h>

#include <QAbstractListModel>
#include <QComboBox>
#include <QPushButton>

namespace Rust {
namespace Internal {

class ToolChainsModel : public QAbstractListModel
{
public:
    ToolChainsModel(const ToolChainManager& toolChainManager, QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int rowForId(Core::Id id) const;
    Core::Id idForRow(int i) const;
    const ToolChain* toolChainForRow(int i) const;

private:
    const ToolChainManager& m_toolChainManager;
};

ToolChainsModel::ToolChainsModel(const ToolChainManager &toolChainManager, QObject *parent)
    : QAbstractListModel(parent),
      m_toolChainManager(toolChainManager)
{
    connect(&m_toolChainManager, &ToolChainManager::toolChainsAboutToBeReset,
            [this]{ beginResetModel(); });

    connect(&m_toolChainManager, &ToolChainManager::toolChainsReset,
            [this]{ endResetModel(); });
}

int ToolChainsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return 1 + m_toolChainManager.toolChains().size();
}

QVariant ToolChainsModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && index.column() == 0 && role == Qt::DisplayRole) {
        if (const ToolChain *toolChain = toolChainForRow(index.row())) {
            if (!toolChain->fullToolChainName.isEmpty()) {
                return QString("%1 - %2").arg(toolChain->name).arg(toolChain->fullToolChainName);
            } else {
                return toolChain->name;
            }
        } else {
            return tr("Default");
        }
    }
    return QVariant();
}

int ToolChainsModel::rowForId(Core::Id id) const
{
    auto it = std::find(m_toolChainManager.toolChains().begin(),
                        m_toolChainManager.toolChains().end(),
                        id);

    if (it != m_toolChainManager.toolChains().end()) {
        const int i = std::distance(m_toolChainManager.toolChains().begin(), it);
        return 1 + i;
    }

    return 0;
}

Core::Id ToolChainsModel::idForRow(int i) const
{
    if (const ToolChain *toolChain = toolChainForRow(i))
        return toolChain->id;
    else
        return Core::Id();
}

const ToolChain *ToolChainsModel::toolChainForRow(int i) const
{
    if (i > 0 && i < m_toolChainManager.toolChains().size()+1) {
        return &m_toolChainManager.toolChains().at(i - 1);
    } else {
        return nullptr;
    }
}

KitConfigWidget::KitConfigWidget(const ToolChainManager& toolChainManager,
                                 ProjectExplorer::Kit *kit,
                                 const ProjectExplorer::KitInformation *ki)
    : ProjectExplorer::KitConfigWidget(kit, ki),
      m_toolChainManager(toolChainManager),
      m_model(new ToolChainsModel(toolChainManager, this)),
      m_comboBox(new QComboBox),
      m_pushButton(new QPushButton(ProjectExplorer::KitConfigWidget::msgManage())),
      m_modelInReset(false)
{
    m_comboBox->setModel(m_model);

    connect(m_comboBox.data(), QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int index) {
        if (!m_modelInReset) {
            KitInformation::setToolChain(m_kit, m_model->idForRow(index));
        }
    });

    connect(m_model, &QAbstractItemModel::modelAboutToBeReset, [this]{ m_modelInReset = true; });
    connect(m_model, &QAbstractItemModel::modelReset, [this]{
        m_modelInReset = false;
        refresh();
    });

    connect(m_pushButton.data(), &QPushButton::clicked,
            [this] { Core::ICore::showOptionsDialog(ToolsOptionsPage::ID, buttonWidget()); });
}

KitConfigWidget::~KitConfigWidget()
{
}

QString KitConfigWidget::displayName() const
{
    return tr("Rust version:");
}

void KitConfigWidget::makeReadOnly()
{
    m_comboBox->setEnabled(false);
    m_pushButton->setEnabled(false);
}

void KitConfigWidget::refresh()
{
    m_comboBox->setCurrentIndex(m_model->rowForId(KitInformation::getToolChain(m_kit)));
}

QWidget *KitConfigWidget::mainWidget() const
{
    return m_comboBox.data();
}

QWidget *KitConfigWidget::buttonWidget() const
{
    return m_pushButton.data();
}

} // namespace Internal
} // namespace Rust
