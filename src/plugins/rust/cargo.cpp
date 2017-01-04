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

#include "cargo.h"

#include <utils/environment.h>

#include <QDir>

namespace Rust {

namespace {
    constexpr char CARGO_BINARY[] = "cargo";

    Utils::FileName cargoBinary()
    {
        QStringList additionalSearchDirs;
        return Utils::Environment::systemEnvironment().searchInPath(
                    QLatin1String(CARGO_BINARY), additionalSearchDirs);
    }
}

Cargo::Cargo(const QString &workingDirectory,
                     const QProcessEnvironment &environment,
                     QObject *parent) :
    QObject(parent),
    m_shellCommand(workingDirectory, environment),
    m_mode(None)
{
}

Cargo::~Cargo()
{
}

void Cargo::getListOfFiles()
{
    QStringList args = {QLatin1String("package"), QLatin1String("--list")};
    run(args);
}

void Cargo::run(QStringList arguments, int timeoutS)
{
    m_shellCommand.addJob(cargoBinary(), arguments, timeoutS);
}

void Cargo::readStdOutText(const QString &text)
{
    m_stdOut += text;
}

void Cargo::readStdErrText(const QString &text)
{
    m_stdErr += text;
}

void Cargo::finish(bool ok, int exitCode, const QVariant &cookie)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(cookie);
    if (ok)
    {
        const QStringList relativePaths = m_stdOut.split('\n', QString::SkipEmptyParts);
        QVector<Utils::FileName> fileList;
        for (QString relativePath : relativePaths)
        {
            QFileInfo fileInfo(QDir(m_shellCommand.defaultWorkingDirectory()), relativePath);
            fileList.push_back(Utils::FileName(fileInfo));
        }
        emit listOfFiles(std::move(fileList));
    }
    else
    {
        emit error(std::move(m_stdErr));
    }

    m_stdOut.clear();
    m_stdErr.clear();
}

}
