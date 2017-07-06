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

#include "rusttargetarchwidget.h"
#include "rusttargetarchinformation.h"
#include "rusttoolchainmanager.h"
#include "rusttoolsoptionspage.h"

#include <coreplugin/icore.h>

#include <QAbstractListModel>
#include <QComboBox>
#include <QPushButton>

namespace Rust {
namespace Internal {

class TargetArchModel : public QAbstractListModel
{
public:
    TargetArchModel(const ToolChainManager& toolChainManager, QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int rowForId(Core::Id id) const;
    Core::Id idForRow(int i) const;
    const TargetArch* targetArchForRow(int i) const;

private:
    const ToolChainManager& m_toolChainManager;
};

TargetArchModel::TargetArchModel(const ToolChainManager &toolChainManager, QObject *parent)
    : QAbstractListModel(parent),
      m_toolChainManager(toolChainManager)
{
    connect(&m_toolChainManager, &ToolChainManager::targetArchsAboutToBeReset,
            [this]{ beginResetModel(); });

    connect(&m_toolChainManager, &ToolChainManager::targetArchsReset,
            [this]{ endResetModel(); });
}

int TargetArchModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return 1 + m_toolChainManager.targetArchs().size();
}

QVariant TargetArchModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && index.column() == 0 && role == Qt::DisplayRole) {
        if (const TargetArch *targetArch = targetArchForRow(index.row())) {
            return targetArch->name;
        } else {
            return tr("Default");
        }
    }
    return QVariant();
}

int TargetArchModel::rowForId(Core::Id id) const
{
    auto it = std::find(m_toolChainManager.targetArchs().begin(),
                        m_toolChainManager.targetArchs().end(),
                        id);

    if (it != m_toolChainManager.targetArchs().end()) {
        const int i = std::distance(m_toolChainManager.targetArchs().begin(), it);
        return 1 + i;
    }

    return 0;
}

Core::Id TargetArchModel::idForRow(int i) const
{
    if (const TargetArch *targetArch = targetArchForRow(i))
        return targetArch->id;
    else
        return Core::Id();
}

const TargetArch *TargetArchModel::targetArchForRow(int i) const
{
    if (i > 0 && i < m_toolChainManager.targetArchs().size()+1) {
        return &m_toolChainManager.targetArchs().at(i - 1);
    } else {
        return nullptr;
    }
}

TargetArchWidget::TargetArchWidget(const ToolChainManager& toolChainManager,
                                 ProjectExplorer::Kit *kit,
                                 const ProjectExplorer::KitInformation *ki)
    : ProjectExplorer::KitConfigWidget(kit, ki),
      m_toolChainManager(toolChainManager),
      m_model(new TargetArchModel(toolChainManager, this)),
      m_comboBox(new QComboBox),
      m_pushButton(new QPushButton(ProjectExplorer::KitConfigWidget::msgManage()))
{
    m_comboBox->setModel(m_model);

    connect(m_comboBox.data(), QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int index) {
        if (!m_modelInReset) {
            TargetArchInformation::setTargetArch(m_kit, m_model->idForRow(index));
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

TargetArchWidget::~TargetArchWidget()
{
}

QString TargetArchWidget::displayName() const
{
    return tr("Rust target architecture:");
}

void TargetArchWidget::makeReadOnly()
{
    m_comboBox->setEnabled(false);
    m_pushButton->setEnabled(false);
}

void TargetArchWidget::refresh()
{
    m_comboBox->setCurrentIndex(m_model->rowForId(TargetArchInformation::getTargetArch(m_kit)));
}

QWidget *TargetArchWidget::mainWidget() const
{
    return m_comboBox.data();
}

QWidget *TargetArchWidget::buttonWidget() const
{
    return m_pushButton.data();
}

} // namespace Internal
} // namespace Rust
