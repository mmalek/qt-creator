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

#include "rustracer.h"

#include <QIcon>
#include <QProcess>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextStream>

namespace Rust {
namespace Internal {
namespace Racer {
namespace {

constexpr int RACER_TIMEOUT_MSEC = 5000;

QLatin1String toString(Request request)
{
    switch(request) {
    case Request::Complete: return QLatin1String("complete");
    case Request::FindDefinition: return QLatin1String("find-definition");
    default: return QLatin1String();
    }
}

} // namespace

QVector<Result> run(Request request, const QTextCursor& cursor, const QString &filePath)
{
    QVector<Result> results;

    const int line = cursor.blockNumber() + 1;
    const int column = cursor.positionInBlock();

    QTemporaryFile file;
    if (file.open()) {
        Q_ASSERT(cursor.document() != nullptr);
        const QTextDocument& textDocument = *cursor.document();
        {
            QTextStream out(&file);
            out.setCodec("UTF-8");
            out << textDocument.toPlainText();
        }

        const QString program = QLatin1String("racer");
        const QStringList arguments = {
            toString(request),
            QString::number(line),
            QString::number(column),
            filePath,
            file.fileName()
        };

        QProcess process;
        process.start(program, arguments);

        if (process.waitForFinished(RACER_TIMEOUT_MSEC)) {
            QString str = QString::fromUtf8(process.readAllStandardOutput());
            QRegularExpression match("^MATCH (\\w+),(\\d+),(\\d+),(.+),(\\w+),(.+)$",
                                     QRegularExpression::MultilineOption);

            for (QRegularExpressionMatchIterator it = match.globalMatch(str); it.hasNext(); ) {
                QRegularExpressionMatch match = it.next();
                if (match.lastCapturedIndex() == 6) {
                    Result result;
                    result.symbol = match.captured(1);
                    result.line = match.captured(2).toInt();
                    result.column = match.captured(3).toInt();
                    result.filePath = match.captured(4);
                    result.type = Result::toType(match.capturedRef(5));
                    result.detail = match.captured(6);
                    results.push_back(std::move(result));
                }
            }
        }
    }

    return results;
}

Result::Type Result::toType(QStringRef text)
{
    if (text == QLatin1String("EnumVariant")) {
        return Type::EnumVariant;
    } else if (text == QLatin1String("Function")) {
        return Type::Function;
    } else if (text == QLatin1String("Module")) {
        return Type::Module;
    } else {
        return Type::Other;
    }
}

QIcon Result::icon(Type type)
{
    switch (type) {
    case Type::EnumVariant: {
        static QIcon icon(QLatin1String(":/codemodel/images/enum.png"));
        return icon;
    }
    case Type::Function: {
        static QIcon icon(QLatin1String(":/codemodel/images/classmemberfunction.png"));
        return icon;
    }
    case Type::Keyword:
    case Type::Module: {
        static QIcon icon(QLatin1String(":/codemodel/images/keyword.png"));
        return icon;
    }
    case Type::Other: {
        static QIcon icon(QLatin1String(":/codemodel/images/member.png"));
        return icon;
    }
    default:
        return QIcon();
    }
}

} // namespace Racer
} // namespace Internal
} // namespace Rust
