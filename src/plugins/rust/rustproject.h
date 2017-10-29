/****************************************************************************
**
** Copyright (C) Filippo Cucchetto <filippocucchetto@gmail.com>
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

#pragma once

#include "rustproduct.h"

#include <projectexplorer/project.h>

#include <QElapsedTimer>
#include <QFutureWatcher>
#include <QFileSystemWatcher>
#include <QList>
#include <QString>
#include <QTimer>
#include <QVector>

namespace ProjectExplorer {
class FileNode;
class FolderNode;
}

namespace TextEditor { class TextDocument; }

namespace Rust {
namespace Internal {

class Project final : public ProjectExplorer::Project
{
    Q_OBJECT

public:
    explicit Project(const Utils::FileName &fileName);

    bool needsConfiguration() const override;
    bool supportsKit(ProjectExplorer::Kit *kit, QString *errorMessage) const override;

    const QVector<Product>& products() const { return m_products; }

private:
    void scheduleProjectScan();
    void collectProjectFiles();
    void updateProject();

    QStringList m_files;
    QFutureWatcher<QList<ProjectExplorer::FileNode *>> m_futureWatcher;
    QElapsedTimer m_lastProjectScan;
    QTimer m_projectScanTimer;
    QFileSystemWatcher m_fileSystemWatcher;
    QVector<Product> m_products;
};

} // namespace Internal
} // namespace Rust
