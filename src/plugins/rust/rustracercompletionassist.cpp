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

#include "rustracercompletionassist.h"
#include "rusteditors.h"

#include <coreplugin/id.h>
#include <texteditor/codeassist/assistinterface.h>
#include <texteditor/codeassist/assistproposalitem.h>
#include <texteditor/codeassist/genericproposal.h>
#include <texteditor/texteditor.h>

#include <QProcess>
#include <QRegularExpression>
#include <QStringList>
#include <QTemporaryFile>
#include <QTextBlock>
#include <QTextStream>

namespace Rust {
namespace Internal {

using namespace TextEditor;

constexpr int RACER_TIMEOUT_MSEC = 5000;

RacerCompletionAssistProcessor::RacerCompletionAssistProcessor()
    : KeywordsCompletionAssistProcessor({})
{
}

RacerCompletionAssistProcessor::~RacerCompletionAssistProcessor()
{
}

IAssistProposal *RacerCompletionAssistProcessor::perform(const AssistInterface *interface)
{
    QTextCursor cursor(interface->textDocument());
    cursor.setPosition(interface->position());

    const QTextBlock block = cursor.block();
    const int line = block.blockNumber() + 1;
    int column = cursor.positionInBlock();

    QTemporaryFile file;
    if (file.open()) {
        {
            QTextStream out(&file);
            out.setCodec("UTF-8");
            out << interface->textDocument()->toPlainText();
        }

        const QString program = QLatin1String("racer");
        const QStringList arguments = {
            QLatin1String("complete"),
            QString::number(line),
            QString::number(column),
            interface->fileName(),
            file.fileName()
        };

        QProcess process;
        process.start(program, arguments);

        if (process.waitForFinished(RACER_TIMEOUT_MSEC)) {
            QString str = QString::fromUtf8(process.readAllStandardOutput());
            QRegularExpression match("^MATCH (\\w+),(\\d+),(\\d+),(.+),(\\w+),(.+)$",
                                     QRegularExpression::MultilineOption);

            QStringList variables;
            QStringList functions;
            QMap<QString, QStringList> functionArgs;
            for (QRegularExpressionMatchIterator it = match.globalMatch(str); it.hasNext(); ) {
                QRegularExpressionMatch match = it.next();
                if (match.lastCapturedIndex() == 6) {
                    QString symbol = match.captured(1);
                    variables.append(symbol);

                    if (match.capturedRef(5) == QLatin1String("Function")) {
                        functions.append(symbol);
                        functionArgs[symbol].append(match.captured(6));
                    }
                }
            }

            Keywords keywords(variables, functions, functionArgs);
            setKeywords(keywords);
        }
    }

    return KeywordsCompletionAssistProcessor::perform(interface);
}

RacerCompletionAssistProvider::RacerCompletionAssistProvider(QObject *parent)
    : CompletionAssistProvider(parent)
{
}

bool RacerCompletionAssistProvider::supportsEditor(Core::Id editorId) const
{
    return editorId == Editors::RUST;
}

IAssistProcessor *RacerCompletionAssistProvider::createProcessor() const
{
    return new RacerCompletionAssistProcessor;
}

int RacerCompletionAssistProvider::activationCharSequenceLength() const
{
    return 2;
}

bool RacerCompletionAssistProvider::isActivationCharSequence(const QString &sequence) const
{
    return sequence.endsWith(QLatin1Char('.')) || sequence == QLatin1String("::");
}

RacerAssistProposalItem::RacerAssistProposalItem(const QString &text,
                                                 const QString &detail,
                                                 RacerAssistProposalItem::Type type)
{
    setText(text);
    setDetail(detail);
    setIcon(iconForType(type));
}

QIcon RacerAssistProposalItem::iconForType(RacerAssistProposalItem::Type type)
{
    switch (type) {
    case Type::Function: {
        static QIcon icon(QLatin1String(":/codemodel/images/classmemberfunction.png"));
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

} // namespace Internal
} // namespace Rust
