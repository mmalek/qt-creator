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

#include "rustcompileroutputparser.h"

#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/task.h>
#include <texteditor/fontsettings.h>
#include <texteditor/texteditorsettings.h>
#include <utils/utilsicons.h>

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QVector>

#include <algorithm>
#include <cmath>

namespace Rust {
namespace Internal {
namespace {

enum class Level {
    Unknown,
    Warning,
    Error,
    Help,
};

struct Code {
    QString code;
    QString explanation;
};

struct Text {
    int highlightEnd = -1;
    int highlightStart = -1;
    QString text;
};

struct Span {
    int byteEnd = -1;
    int byteStart = -1;
    int columnEnd = -1;
    int columnStart = -1;
    Utils::FileName fileName;
    bool isPrimary = false;
    QString label;
    int lineEnd = -1;
    int lineStart = -1;
    QVector<Text> text;
};

struct Message {
    QVector<Message> children;
    Level level = Level::Unknown;
    QString message;
    QVector<Span> spans;
};

Level parseLevel(QString level)
{
    if (level.startsWith("error")) {
        return Level::Error;
    } else if (level == "warning") {
        return Level::Warning;
    } else if (level == "help") {
        return Level::Help;
    } else {
        return Level::Unknown;
    }
}

ProjectExplorer::Task::TaskType toTaskType(Level level)
{
    switch (level) {
    case Level::Warning: return ProjectExplorer::Task::Warning;
    case Level::Error: return ProjectExplorer::Task::Error;
    default: return ProjectExplorer::Task::Unknown;
    }
}

QIcon toIcon(Level level)
{
    switch (level) {
    case Level::Warning: return Utils::Icons::WARNING.icon();
    case Level::Error: return Utils::Icons::ERROR.icon();
    case Level::Help: return Utils::Icons::INFO.icon();
    default: return QIcon();
    }
}

Text parseText(const QJsonObject& text)
{
    Text result;
    result.highlightEnd = text.value("highlight_end").toInt(-1);
    result.highlightStart = text.value("highlight_start").toInt(-1);
    result.text = text.value("text").toString();
    return result;
}

Span parseSpan(const QJsonObject& span)
{
    Span result;
    result.byteEnd = span.value("byte_end").toInt(-1);
    result.byteStart = span.value("byte_start").toInt(-1);
    result.columnEnd = span.value("column_end").toInt(-1);
    result.columnStart = span.value("column_start").toInt(-1);

    const QString fileName = span.value("file_name").toString();
    if (!fileName.isNull()) {
        result.fileName = Utils::FileName::fromString(QDir::cleanPath(fileName));
    }

    result.isPrimary = span.value("is_primary").toBool();
    result.label = span.value("label").toString();
    result.lineEnd = span.value("line_end").toInt(-1);
    result.lineStart = span.value("line_start").toInt(-1);

    for (const QJsonValue value : span.value("text").toArray()) {
        result.text.push_back(parseText(value.toObject()));
    }

    return result;
}

Message parseMessage(const QJsonObject& message)
{
    Message result;
    result.level = parseLevel(message.value("level").toString());
    result.message = message.value("message").toString();

    for (const QJsonValue value : message.value("spans").toArray()) {
        result.spans.push_back(parseSpan(value.toObject()));
    }

    for (const QJsonValue value : message.value("children").toArray()) {
        result.children.push_back(parseMessage(value.toObject()));
    }

    std::sort(result.spans.begin(), result.spans.end(),
              [](const Span& lhs, const Span& rhs) {
        return (lhs.lineStart == rhs.lineStart)
                ? (lhs.lineEnd < rhs.lineEnd)
                : (lhs.lineStart < rhs.lineStart);
    });

    return result;
}

void formatDescription(const Message& message,
                       QString& description,
                       QVector<QTextLayout::FormatRange>& formats)
{
    if (!description.isEmpty() && !message.message.isEmpty()) {
        description.append(QLatin1Char('\n'));
    }

    description.append(message.message);

    const TextEditor::FontSettings& fontSettings = TextEditor::TextEditorSettings::fontSettings();

    QTextCharFormat codeFormat;
    codeFormat.setFont(QFont(fontSettings.family()));

    QTextCharFormat warningFormat = fontSettings.toTextCharFormat(TextEditor::C_WARNING);
    QTextCharFormat codeHighlightFormat = codeFormat;
    codeHighlightFormat.setFontUnderline(true);
    codeHighlightFormat.setFontWeight(QFont::Bold);
    codeHighlightFormat.setUnderlineStyle(warningFormat.underlineStyle());
    codeHighlightFormat.setUnderlineColor(warningFormat.underlineColor());

    QTextCharFormat errorFormat = fontSettings.toTextCharFormat(TextEditor::C_ERROR);
    QTextCharFormat primaryHighlightFormat = codeHighlightFormat;
    primaryHighlightFormat.setUnderlineStyle(errorFormat.underlineStyle());
    primaryHighlightFormat.setUnderlineColor(errorFormat.underlineColor());

    int lineDigits = 0;
    for (const Span& span : message.spans) {
        for (const Text& text : span.text) {
            if (!text.text.isEmpty()) {
                const int curDigits = std::ceil(std::log10(span.lineEnd));
                lineDigits = std::max(lineDigits, curDigits);
            }
        }
    }

    const int prefixCount = lineDigits + 3;

    int prevLine = -1;

    for (const Span& span : message.spans) {
        for (const Text& text : span.text) {

            if (prevLine >= 0 && span.lineStart > prevLine + 1) {
                formats.append({description.length() + 1, 3, codeFormat});
                description.append(QLatin1String("\n..."));
            }

            prevLine = span.lineEnd;

            if (!text.text.isEmpty()) {
                formats.append({description.length(),
                                prefixCount + text.text.length(),
                                codeFormat});

                const int highlightLength = text.highlightEnd - text.highlightStart;
                if (highlightLength > 0) {
                    formats.append({description.length() + prefixCount + text.highlightStart,
                                    highlightLength,
                                    span.isPrimary ? primaryHighlightFormat : codeHighlightFormat});
                }

                description.append(QLatin1Char('\n'));
                description.append(QString("%1 | %2")
                                   .arg(span.lineStart, lineDigits, 10, QLatin1Char(' '))
                                   .arg(text.text));

                if (span.columnStart > 0 && !span.label.isEmpty()) {
                    description.append(QLatin1Char('\n'));
                    formats.append({description.length(),
                                    prefixCount + span.columnStart - 1,
                                    codeFormat});
                    description.append(QString("%1 | %2")
                                       .arg(QString(lineDigits, QLatin1Char(' ')))
                                       .arg(span.label, span.columnStart - 1 + span.label.size(),
                                            QLatin1Char(' ')));
                }
            }
        }
    }

    if (!message.children.isEmpty()) {
        description.append(QLatin1Char('\n'));
        for (const Message& child : message.children) {
            formatDescription(child, description, formats);
        }
    }
}

ProjectExplorer::Task toTask(Message message)
{
    ProjectExplorer::Task task(toTaskType(message.level),
                               QString(),
                               Utils::FileName(),
                               -1,
                               ProjectExplorer::Constants::TASK_CATEGORY_COMPILE);

    task.icon = toIcon(message.level);

    formatDescription(message, task.description, task.formats);

    auto primarySpan = std::find_if(message.spans.begin(),
                                    message.spans.end(),
                                    [](const Span& span) { return span.isPrimary; });

    if (primarySpan != message.spans.end()) {
        task.file = primarySpan->fileName;
        task.line = primarySpan->lineStart;
    }

    return task;
}

} // namespace

void CompilerOutputParser::stdOutput(const QString &line)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(line.toUtf8(), &parseError);
    if (!document.isNull()) {
        QJsonObject root = document.object();
        QJsonObject message = root.value("message").toObject();

        emit addTask(toTask(parseMessage(message)));
    } else {
        emit addTask({ProjectExplorer::Task::Error,
                      tr("Internal error: cannot parse message: %1").arg(parseError.errorString()),
                      Utils::FileName(), -1,
                      ProjectExplorer::Constants::TASK_CATEGORY_COMPILE});
    }
}

bool CompilerOutputParser::isParsable(const QString &line)
{
    return line.startsWith(QLatin1Char('{'));
}

} // namespace Internal
} // namespace Rust
