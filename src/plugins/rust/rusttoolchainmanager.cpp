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

#include "rusttoolchainmanager.h"

#include <utils/environment.h>
#include <utils/fileutils.h>

#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>

namespace Rust {
namespace Internal {

namespace {

#ifdef Q_OS_WIN
#define MAKE_BINARY_NAME(name) name ".exe"
#else
#define MAKE_BINARY_NAME(name) name
#endif

Q_CONSTEXPR QLatin1String CARGO_BINARY{MAKE_BINARY_NAME("cargo")};
Q_CONSTEXPR QLatin1String RACER_BINARY{MAKE_BINARY_NAME("racer")};

constexpr int ONE_SECOND = 1000;

Utils::FileName searchInDirectory(const QStringList &execs, QString directory)
{
    const QChar slash = QLatin1Char('/');
    if (directory.isEmpty())
        return Utils::FileName();
    // Avoid turing / into // on windows which triggers windows to check
    // for network drives!
    if (!directory.endsWith(slash))
        directory += slash;

    foreach (const QString &exec, execs) {
        QFileInfo fi(directory + exec);
        if (fi.exists() && fi.isFile() && fi.isExecutable())
            return Utils::FileName::fromString(fi.absoluteFilePath());
    }
    return Utils::FileName();
}

void addUniquely(QVector<ToolChain>& toolChains, ToolChain newTc)
{
    if (std::none_of(toolChains.begin(),
                     toolChains.end(),
                     [&newTc] (const ToolChain& tc) { return newTc.id == tc.id; })) {
        toolChains.push_back(std::move(newTc));
    }
}

} // namespace

ToolChainManager::ToolChainManager(QObject *parent) : QObject(parent)
{
    QStringList dirs = Utils::Environment::systemEnvironment().path();
    dirs.removeDuplicates();

    for (const QString& dir : dirs) {
        Utils::FileName file = searchInDirectory({CARGO_BINARY}, dir);
        if (!file.isNull()) {
            QProcess process;
            process.start(file.toString(), {QLatin1String("--version")});
            if (process.waitForFinished(ONE_SECOND)) {
                const QString version = QString::fromLocal8Bit(process.readLine().trimmed());

                QRegularExpression regex("^cargo (?<name>(?<version>\\S+) \\((?<id>\\w+) \\S+\\))$");
                QRegularExpressionMatch match = regex.match(version);
                if (match.hasMatch()) {
                    ToolChain toolChain;
                    toolChain.id = Core::Id::fromString(match.captured("id"));
                    toolChain.name = match.captured("name");
                    toolChain.version = match.captured("version");
                    toolChain.path = Utils::FileName::fromString(dir);
                    toolChain.cargoPath = file;
                    addUniquely(m_autodetected, std::move(toolChain));
                }
            }
        }
    }
}

const ToolChain *ToolChainManager::get(Core::Id id) const
{
    auto findToolChain = [](const QVector<ToolChain>& toolChains, const Core::Id& id)
    {
        auto it = std::find_if(toolChains.cbegin(),
                               toolChains.cend(),
                               [&id](const ToolChain& toolChain) { return toolChain.id == id; });

        return (it != toolChains.cend()) ? &(*it) : nullptr;
    };

    if (const ToolChain* toolChain = findToolChain(m_autodetected, id)) {
        return toolChain;
    } else if (const ToolChain* toolChain = findToolChain(m_manual, id)) {
        return toolChain;
    } else {
        return nullptr;
    }
}

const ToolChain *ToolChainManager::getFirst() const
{
    if (!m_autodetected.isEmpty())
        return &m_autodetected.front();
    else if (!m_manual.isEmpty())
        return &m_manual.front();
    else
        return nullptr;
}

} // namespace Internal
} // namespace Rust
