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

#include <utils/icon.h>

#include <QIcon>
#include <QProcess>
#include <QRegularExpression>
#include <QScopedPointer>
#include <QTemporaryFile>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextStream>

#include <array>
#include <algorithm>

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

constexpr std::size_t NUM_RESULT_TYPES = static_cast<std::size_t>(Result::Type::NumResultTypes);

Q_CONSTEXPR std::array<QLatin1String, NUM_RESULT_TYPES> RESULT_TYPE_NAMES =
{
    QLatin1String{"Struct"},
    QLatin1String{"Module"},
    QLatin1String{"MatchArm"},
    QLatin1String{"Function"},
    QLatin1String{"Crate"},
    QLatin1String{"Let"},
    QLatin1String{"IfLet"},
    QLatin1String{"WhileLet"},
    QLatin1String{"For"},
    QLatin1String{"StructField"},
    QLatin1String{"Impl"},
    QLatin1String{"TraitImpl"},
    QLatin1String{"Enum"},
    QLatin1String{"EnumVariant"},
    QLatin1String{"Type"},
    QLatin1String{"FnArg"},
    QLatin1String{"Trait"},
    QLatin1String{"Const"},
    QLatin1String{"Static"},
    QLatin1String{"Macro"},
    QLatin1String{"Builtin"}
};

} // namespace

QVector<Result> run(Request request, const QTextCursor& cursor, const QString &filePath)
{
    const int line = cursor.blockNumber() + 1;
    const int column = cursor.positionInBlock();

    Q_ASSERT(cursor.document() != nullptr);
    const QTextDocument& textDocument = *cursor.document();

    QScopedPointer<QTemporaryFile> substituteFile;
    if (textDocument.isModified()) {
        substituteFile.reset(new QTemporaryFile);
        if (substituteFile->open()) {
            QTextStream out(substituteFile.data());
            out.setCodec("UTF-8");
            out << textDocument.toPlainText();
        } else {
            return {};
        }
    }

    const QString program = QLatin1String("racer");
    QStringList arguments = {
        toString(request),
        QString::number(line),
        QString::number(column),
        filePath
    };

    if (substituteFile) {
        arguments.push_back(substituteFile->fileName());
    }

    QVector<Result> results;

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

    return results;
}

Result::Type Result::toType(QStringRef text)
{
    for (std::size_t i = 0; i < RESULT_TYPE_NAMES.size(); ++i) {
        if (RESULT_TYPE_NAMES.at(i) == text) {
            return static_cast<Result::Type>(i);
        }
    }
    return Type::NumResultTypes;
}

QIcon Result::icon(Type type)
{
    static const Utils::IconMaskAndColor classRelationIcon {
        QLatin1String(":/codemodel/images/classrelation.png"),
        Utils::Theme::IconsCodeModelOverlayForegroundColor};
    static const Utils::IconMaskAndColor classRelationBackgroundIcon {
        QLatin1String(":/codemodel/images/classrelationbackground.png"),
        Utils::Theme::IconsCodeModelOverlayBackgroundColor};
    static const Utils::IconMaskAndColor classMemberFunctionIcon {
        QLatin1String(":/codemodel/images/classmemberfunction.png"),
        Utils::Theme::IconsCodeModelFunctionColor};
    static const Utils::IconMaskAndColor classMemberVariableIcon {
        QLatin1String(":/codemodel/images/classmembervariable.png"),
        Utils::Theme::IconsCodeModelVariableColor};
    static const Utils::IconMaskAndColor variableIcon {
        QLatin1String(":/codemodel/images/member.png"),
        Utils::Theme::IconsCodeModelVariableColor};
    static const Utils::IconMaskAndColor staticIcon {
        QLatin1String(":/codemodel/images/static.png"),
        Utils::Theme::IconsCodeModelOverlayForegroundColor};
    static const Utils::IconMaskAndColor staticBackgroundIcon {
        QLatin1String(":/codemodel/images/staticbackground.png"),
        Utils::Theme::IconsCodeModelOverlayBackgroundColor};

    switch (type) {
    case Type::Module:
    case Type::Crate: {
        const static QIcon icon(Utils::Icon({
            {QLatin1String(":/utils/images/namespace.png"),
             Utils::Theme::IconsCodeModelKeywordColor}
        }, Utils::Icon::Tint).icon());
        return icon;
    }
    case Type::Function: {
        const static QIcon icon(Utils::Icon({
            {QLatin1String(":/codemodel/images/member.png"),
             Utils::Theme::IconsCodeModelFunctionColor}
        }, Utils::Icon::Tint).icon());
        return icon;
    }
    case Type::Enum: {
        const static QIcon icon(Utils::Icon({
            {QLatin1String(":/codemodel/images/enum.png"),
             Utils::Theme::IconsCodeModelEnumColor}
        }, Utils::Icon::Tint).icon());
        return icon;
    }
    case Type::EnumVariant: {
        const static QIcon icon(Utils::Icon({
            {QLatin1String(":/codemodel/images/enumerator.png"),
             Utils::Theme::IconsCodeModelEnumColor}
        }, Utils::Icon::Tint).icon());
        return icon;
    }
    case Type::Macro: {
        const static QIcon icon(Utils::Icon({
            {QLatin1String(":/codemodel/images/macro.png"),
             Utils::Theme::IconsCodeModelMacroColor}
        }, Utils::Icon::Tint).icon());
        return icon;
    }
    case Type::Struct: {
        const static QIcon icon(Utils::Icon({
            classRelationBackgroundIcon, classRelationIcon,
            {QLatin1String(":/codemodel/images/classparent.png"),
             Utils::Theme::IconsCodeModelStructColor},
            classMemberFunctionIcon, classMemberVariableIcon
        }, Utils::Icon::Tint).icon());
        return icon;
    }
    case Type::Trait: {
        const static QIcon icon(Utils::Icon({
            classRelationBackgroundIcon, classRelationIcon,
            {QLatin1String(":/codemodel/images/classparent.png"),
             Utils::Theme::IconsCodeModelClassColor}
        }, Utils::Icon::Tint).icon());
        return icon;
    }
    case Type::MatchArm:
    case Type::Let:
    case Type::IfLet:
    case Type::WhileLet:
    case Type::For:
    case Type::StructField:
    case Type::FnArg: {
        const static QIcon icon(Utils::Icon({
            variableIcon
        }, Utils::Icon::Tint).icon());
        return icon;
    }
    case Type::Const:
    case Type::Static: {
        const static QIcon icon(Utils::Icon({
            variableIcon, staticBackgroundIcon, staticIcon
        }, Utils::Icon::Tint).icon());
        return icon;
    }
    case Type::Impl: {
        const static QIcon icon(Utils::Icon({
            {QLatin1String(":/codemodel/images/slot.png"),
             Utils::Theme::IconsCodeModelStructColor}
        }, Utils::Icon::Tint).icon());
        return icon;
    }
    case Type::TraitImpl: {
        const static QIcon icon(Utils::Icon({
            {QLatin1String(":/codemodel/images/slot.png"),
             Utils::Theme::IconsCodeModelClassColor}
        }, Utils::Icon::Tint).icon());
        return icon;
    }
    case Type::Type:
    case Type::Builtin: {
        const static QIcon icon(Utils::Icon({
            {QLatin1String(":/codemodel/images/keyword.png"),
             Utils::Theme::IconsCodeModelKeywordColor}
        }, Utils::Icon::Tint).icon());
        return icon;
    }
    default:
        return QIcon();
    }
}

} // namespace Racer
} // namespace Internal
} // namespace Rust
