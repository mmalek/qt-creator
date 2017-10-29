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

#include "rustmanifest.h"
#include "rustsettings.h"
#include "rusttoolchainmanager.h"

#include <utils/environment.h>
#include <utils/fileutils.h>
#include <utils/qtcprocess.h>

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>

namespace Rust {
namespace Internal {

namespace {
constexpr int FIVE_SECONDS = 5000;
}

Manifest Manifest::read(const Utils::FileName &manifestFile,
                        QString *errorString)
{
    Manifest manifest;

    QFileInfo fileInfo = manifestFile.toFileInfo();

    if (!fileInfo.isFile()) {
        if (errorString) {
            *errorString = tr("Not a file");
        }
        return {};
    }

    manifest.file = Utils::FileName::fromString(fileInfo.absoluteFilePath());
    manifest.directory = Utils::FileName::fromString(fileInfo.absolutePath());

    Utils::QtcProcess cargo;
    cargo.setCommand(Settings::value(Settings::CARGO), QLatin1String("read-manifest"));
    cargo.setEnvironment(ToolChainManager::makeEnvironment());
    cargo.setWorkingDirectory(fileInfo.absolutePath());
    cargo.start();
    if (!cargo.waitForFinished(FIVE_SECONDS)) {
        if (errorString) {
            *errorString = tr("%1 not found");
        }
        return {};
    } else if (cargo.exitStatus() != QProcess::NormalExit) {
        if (errorString) {
            *errorString = tr("%1 crashed");
        }
        return {};
    } else if (cargo.exitCode() != 0) {
        if (errorString) {
            *errorString = tr("%1: %2").arg(manifestFile.toString())
                                       .arg(QString::fromLocal8Bit(cargo.readAllStandardError()));
        }
        return {};
    }

    QByteArray output = cargo.readAllStandardOutput();

    const QJsonDocument document = QJsonDocument::fromJson(output);
    if (!document.isNull() && document.isObject()) {
        const QJsonObject root = document.object();
        manifest.name = root.value(QLatin1String("name")).toString();

        for (QJsonValue targetVal : root.value(QLatin1String("targets")).toArray()) {
            const QJsonObject target = targetVal.toObject();
            const QJsonArray kindArr = target.value(QLatin1String("kind")).toArray();
            if (!kindArr.empty()) {
                const QString name = target.value(QLatin1String("name")).toString();
                const QString srcPathStr = target.value(QLatin1String("src_path")).toString();
                const Utils::FileName srcPath = Utils::FileName::fromString(srcPathStr);
                const QString kindStr = kindArr.first().toString();
                if (kindStr == QLatin1String("bench")) {
                    manifest.products.append(Product{Product::Benchmark, name, srcPath});
                } else if (kindStr == QLatin1String("bin")) {
                    manifest.products.append(Product{Product::Binary, name, srcPath});
                } else if (kindStr == QLatin1String("example")) {
                    manifest.products.append(Product{Product::Example, name, srcPath});
                } else if (kindStr == QLatin1String("lib")) {
                    manifest.products.append(Product{Product::Library, name, srcPath});
                } else  if (kindStr == QLatin1String("test")) {
                    manifest.products.append(Product{Product::Test, name, srcPath});
                }
            }
        }
    }

    if (!manifest.name.isEmpty()) {
        return manifest;
    } else {
        *errorString = tr("Invalid manifest file");
        return {};
    }
}

} // namespace Internal
} // namespace Rust
