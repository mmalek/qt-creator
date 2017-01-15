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

#include "rustcparser.hpp"

#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/task.h>
#include <texteditor/fontsettings.h>
#include <texteditor/texteditorsettings.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>

namespace Rust {

void RustcParser::stdOutput(const QString &line)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(line.toUtf8(), &parseError);
    if (document.isNull()) {
        addTask({ProjectExplorer::Task::Error,
                 tr("Internal error: cannot parse message: %1").arg(parseError.errorString()),
                 Utils::FileName(), -1,
                 ProjectExplorer::Constants::TASK_CATEGORY_COMPILE});
        return;
    }

    QJsonObject root = document.object();

    parseMessage(root.value("message").toObject());
}

void RustcParser::parseMessage(const QJsonObject& message)
{
    QString description = message.value("message").toString();

    QString level = message.value("level").toString();

    ProjectExplorer::Task::TaskType taskType;
    if (level.startsWith("error")) {
        taskType = ProjectExplorer::Task::Error;
    } else if (level == "warning") {
        taskType = ProjectExplorer::Task::Warning;
    } else {
        taskType = ProjectExplorer::Task::Unknown;
    }

    Utils::FileName primaryFileName;
    int primaryFileNameLine = -1;
    QVector<QTextLayout::FormatRange> formats;

    for (QJsonValue spanVal : message.value("spans").toArray())
    {
        const QJsonObject span = spanVal.toObject();

        const bool isPrimary = span.value("is_primary").toBool();
        const QString fileName = span.value("file_name").toString();
        const int lineStart = span.value("line_start").toInt(-1);

        if (isPrimary && !fileName.isNull()) {
            primaryFileName = Utils::FileName::fromString(fileName);
            primaryFileNameLine = lineStart;
        }

        for (const QJsonValue textVal : span.value("text").toArray()) {
            const QJsonObject text = textVal.toObject();
            const int highlightStart = text.value("highlight_start").toInt(-1);
            const int highlightEnd = text.value("highlight_end").toInt(-1);
            const QString textMsg = text.value("text").toString();

            if (!textMsg.isEmpty()) {
                QTextLayout::FormatRange formatRange;
                formatRange.start = description.length();
                formatRange.length = textMsg.length();
                formatRange.format.setFont(TextEditor::TextEditorSettings::fontSettings().font());
                formatRange.format.setFontStyleHint(QFont::Monospace);
                formats.append(formatRange);
                if (highlightStart >= 0 && highlightEnd > highlightStart) {
                    formatRange.start = description.length() + highlightStart;
                    formatRange.length = highlightEnd - highlightStart;
                    formatRange.format.setFontUnderline(true);
                    formatRange.format.setFontWeight(QFont::Bold);
                    formatRange.format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
                    formats.append(formatRange);
                }

                description.reserve(description.size() + textMsg.size() + 1);
                description.append(QLatin1Char('\n'));
                description.append(textMsg);
            }
        }
    }

    ProjectExplorer::Task task(taskType,
             description,
             primaryFileName,
             primaryFileNameLine,
             ProjectExplorer::Constants::TASK_CATEGORY_COMPILE);
    task.formats = std::move(formats);
    addTask(task);

    for (QJsonValue childVal : message.value("children").toArray()) {
        parseMessage(childVal.toObject());
    }

    parseCode(message.value("code").toObject(), primaryFileName, primaryFileNameLine);
}

void RustcParser::parseCode(const QJsonObject& code, const Utils::FileName &file, int line)
{
    if (code.isEmpty()) {
        return;
    }

    ProjectExplorer::Task task(ProjectExplorer::Task::Unknown,
                               QString("%1 %2")
                                 .arg(code.value("code").toString())
                                 .arg(code.value("explanation").toString().trimmed()),
                               file,
                               line,
                               ProjectExplorer::Constants::TASK_CATEGORY_COMPILE);

    addTask(task);
}

} // namespace Rust
