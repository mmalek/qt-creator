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
#include "rustsettings.h"

#include <utils/environment.h>
#include <utils/fileutils.h>

#include <QDir>
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

ToolChain parseCargoVersion(QString output)
{
    ToolChain toolChain;
    QRegularExpression regex("^cargo (?<name>(?<version>\\S+) \\((?<id>\\w+) \\S+\\))$");
    QRegularExpressionMatch match = regex.match(output);
    if (match.hasMatch()) {
        toolChain.id = Core::Id::fromString(match.captured("id"));
        toolChain.name = match.captured("name");
        toolChain.version = match.captured("version");
    }
    return toolChain;
}

ToolChain detectCargo(const QString& dir, const Utils::Environment& environment)
{
    Utils::FileName file = searchInDirectory({CARGO_BINARY}, dir);
    if (!file.isNull()) {
        QProcess cargo;
        cargo.setEnvironment(environment.toStringList());
        cargo.start(file.toString(), {QLatin1String("--version")});
        if (cargo.waitForFinished(ONE_SECOND) && cargo.exitStatus() == QProcess::NormalExit) {
            const QString cargoVersionOutput = QString::fromLocal8Bit(cargo.readLine().trimmed());
            if (ToolChain toolChain = parseCargoVersion(cargoVersionOutput)) {
                toolChain.cargoPath = file;
                return toolChain;
            }
        }
    }

    return {};
}

QVector<ToolChain> parseToolChainList(QStringList toolChainList, const Utils::Environment& environment)
{
    QVector<ToolChain> result;
    for (const QString& toolChainElement : toolChainList) {
        QRegularExpression regex("^(?<full>[\\w*|\\-|\\.]*)(?<default> \\(default\\))?$");
        QRegularExpressionMatch match = regex.match(toolChainElement);
        if (match.hasMatch()) {
            const QString fullToolChainName = match.captured("full");
            const bool isDefault = !match.captured("default").isEmpty();

            QProcess rustup;
            rustup.setEnvironment(environment.toStringList());
            rustup.start(Settings::value(Settings::RUSTUP),
                         {QLatin1String("run"),
                          fullToolChainName,
                          QLatin1String("cargo"),
                          QLatin1String("--version")});

            if (rustup.waitForFinished(ONE_SECOND) && rustup.exitStatus() == QProcess::NormalExit) {
                const QString cargoVersionOutput = QString::fromLocal8Bit(rustup.readLine().trimmed());
                if (ToolChain toolChain = parseCargoVersion(cargoVersionOutput)) {
                    toolChain.fullToolChainName = fullToolChainName;
                    toolChain.isDefault = isDefault;
                    result.push_back(std::move(toolChain));
                }
            }
        }

    }
    return result;
}

QVector<TargetArch> getTargetArchs(const Utils::Environment& environment)
{
    QVector<TargetArch> targetArchs;

    QProcess rustup;
    rustup.setEnvironment(environment.toStringList());
    rustup.start(Settings::value(Settings::RUSTUP),
                 {QLatin1String("target"), QLatin1String("list")});

    if (rustup.waitForFinished(ONE_SECOND) && rustup.exitStatus() == QProcess::NormalExit) {

        while (!rustup.atEnd()) {
            const QString line = QString::fromLocal8Bit(rustup.readLine().trimmed());

            QRegularExpression regex("^(?<name>[\\w*|\\-|\\.]*) \\((?<state>default|installed)\\)$");
            QRegularExpressionMatch match = regex.match(line);
            if (match.hasMatch()) {
                TargetArch targetArch;
                targetArch.name = match.captured("name");
                targetArch.id = Core::Id::fromString(targetArch.name);
                QString state = match.captured("state");
                targetArch.isDefault = (state == QLatin1String("default"));
                targetArchs.push_back(std::move(targetArch));
            }
        }
    }

    return targetArchs;
}

} // namespace

ToolChainManager::ToolChainManager(QObject *parent)
    : QObject(parent),
      m_environment(Utils::Environment::systemEnvironment())
{
    addToEnvironment(m_environment);

    settingsChanged();
}

const ToolChain *ToolChainManager::toolChain(Core::Id id) const
{
    auto it = std::find(m_toolChains.cbegin(), m_toolChains.cend(), id);
    return (it != m_toolChains.cend()) ? &(*it) : nullptr;
}

const ToolChain *ToolChainManager::defaultToolChain() const
{
    if (!m_toolChains.isEmpty()) {
        for (const ToolChain& toolChain : m_toolChains) {
            if (toolChain.isDefault)
                return &toolChain;
        }
        return &m_toolChains.front();
    } else {
        return nullptr;
    }
}

const TargetArch *ToolChainManager::targetArch(Core::Id id) const
{
    auto it = std::find(m_targetArchs.cbegin(), m_targetArchs.cend(), id);
    return (it != m_targetArchs.cend()) ? &(*it) : nullptr;
}

const TargetArch *ToolChainManager::defaultTargetArch() const
{
    if (!m_targetArchs.isEmpty()) {
        for (const TargetArch& targetArch : m_targetArchs) {
            if (targetArch.isDefault)
                return &targetArch;
        }
        return &m_targetArchs.front();
    } else {
        return nullptr;
    }
}

void ToolChainManager::addToEnvironment(Utils::Environment &environment)
{
    environment.appendOrSetPath(QDir::home().filePath(QLatin1String(".cargo/bin")));
}

void ToolChainManager::settingsChanged()
{
    emit toolChainsAboutToBeReset();

    QProcess rustup;
    rustup.setEnvironment(m_environment.toStringList());
    rustup.start(Settings::value(Settings::RUSTUP),
                 {QLatin1String("toolchain"), QLatin1String("list")});

    if (rustup.waitForFinished(ONE_SECOND) && rustup.exitStatus() == QProcess::NormalExit) {
        QStringList toolChainList;

        while (!rustup.atEnd()) {
            toolChainList.push_back(QString::fromLocal8Bit(rustup.readLine().trimmed()));
        }

        m_toolChains = parseToolChainList(toolChainList, m_environment);
    } else {
        QStringList dirs = m_environment.path();
        dirs.removeDuplicates();

        m_toolChains.clear();
        for (const QString& dir : dirs) {
            if (ToolChain toolChain = detectCargo(dir, m_environment)) {
                m_toolChains = {toolChain};
                break;
            }
        }
    }

    emit toolChainsReset();

    emit targetArchsAboutToBeReset();

    m_targetArchs = getTargetArchs(m_environment);

    emit targetArchsReset();
}

} // namespace Internal
} // namespace Rust
