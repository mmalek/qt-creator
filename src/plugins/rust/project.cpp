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

#include "project.h"
#include "projectnode.h"
#include "projectmanager.h"

#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/kit.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/target.h>
#include <texteditor/textdocument.h>

#include <utils/algorithm.h>

#include <QFileInfo>
#include <QQueue>

using namespace ProjectExplorer;
using namespace Utils;

namespace Rust {

const int MIN_TIME_BETWEEN_PROJECT_SCANS = 4500;

Project::Project(ProjectManager *projectManager, const QString &fileName)
{
    setId("Rust.Project");
    setProjectManager(projectManager);
    setDocument(new TextEditor::TextDocument);
    document()->setFilePath(FileName::fromString(fileName));
    QFileInfo fi = QFileInfo(fileName);
    QDir dir = fi.dir();
    setRootProjectNode(new ProjectNode(FileName::fromString(dir.absolutePath())));
    rootProjectNode()->setDisplayName(dir.dirName());

    m_projectScanTimer.setSingleShot(true);
    connect(&m_projectScanTimer, &QTimer::timeout, this, &Project::populateProject);

    populateProject();

    connect(&m_fsWatcher, &QFileSystemWatcher::directoryChanged, this, &Project::scheduleProjectScan);
}

QString Project::displayName() const
{
    return rootProjectNode()->displayName();
}

QStringList Project::files(FilesMode) const
{
    return QStringList(m_files.toList());
}

bool Project::needsConfiguration() const
{
    return targets().empty();
}

void Project::scheduleProjectScan()
{
    auto elapsedTime = m_lastProjectScan.elapsed();
    if (elapsedTime < MIN_TIME_BETWEEN_PROJECT_SCANS) {
        if (!m_projectScanTimer.isActive()) {
            m_projectScanTimer.setInterval(MIN_TIME_BETWEEN_PROJECT_SCANS - elapsedTime);
            m_projectScanTimer.start();
        }
    } else {
        populateProject();
    }
}

void Project::populateProject()
{
    m_lastProjectScan.start();

    QSet<QString> oldFiles = m_files;
    m_files.clear();
    recursiveScanDirectory(QDir(projectDirectory().toString()), m_files);

    if (m_files == oldFiles)
        return;

    QList<FileNode *> fileNodes = Utils::transform(m_files.toList(), [](const QString &f) {
        return new FileNode(FileName::fromString(f), SourceType, false);
    });
    rootProjectNode()->buildTree(fileNodes);

    emit fileListChanged();

    emit parsingFinished();
}

void Project::recursiveScanDirectory(const QDir &dir, QSet<QString> &container)
{
    for (const QFileInfo &info : dir.entryInfoList(QDir::AllDirs |
                                                   QDir::Files |
                                                   QDir::NoDotAndDotDot |
                                                   QDir::NoSymLinks |
                                                   QDir::CaseSensitive)) {
        if (info.isDir())
            recursiveScanDirectory(QDir(info.filePath()), container);
        else
            container << info.filePath();
    }
    m_fsWatcher.addPath(dir.absolutePath());
}

bool Project::supportsKit(Kit *k, QString *) const
{
    return k->isValid();
}

FileNameList Project::files() const
{
    FileNameList result;

    QQueue<FolderNode *> folders;
    folders.enqueue(rootProjectNode());

    while (!folders.isEmpty()) {
        FolderNode *folder = folders.takeFirst();
        for (FileNode *file : folder->fileNodes()) {
            if (file->displayName().endsWith(QLatin1String(".rs")))
                result.append(file->filePath());
        }
        folders.append(folder->subFolderNodes());
    }

    return result;
}

}
