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

#pragma once

#include <QObject>
#include <QQueue>

#include <coreplugin/shellcommand.h>
#include <utils/fileutils.h>

namespace Core { class ShellCommand; }

namespace Rust {
namespace Internal {

class Cargo : public QObject
{
    Q_OBJECT
public:
    Cargo(const QString &workingDirectory,
              const QProcessEnvironment &environment,
              QObject *parent = 0);
    ~Cargo();

    static Utils::FileName binary();

signals:
    void listOfFiles(QVector<Utils::FileName> files);
    void error(const QString& text);

public slots:
    void getListOfFiles();

private slots:
    void readStdOutText(const QString &text);
    void readStdErrText(const QString &text);
    void finish(bool ok, int exitCode, const QVariant &cookie);

private:
    void run(QStringList arguments, int timeoutS = -1);

    enum Mode {
        None,
        ListOfFiles
    };

    Core::ShellCommand m_shellCommand;
    Mode m_mode;
    QString m_stdOut;
    QString m_stdErr;
};

} // namespace Internal
} // namespace Rust
