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
#include "rustsourcelayout.h"

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
namespace {

constexpr int RACER_TIMEOUT_MSEC = 5000;
constexpr int MIN_TYPED_CHARS_AUTOCOMPLETE = 3;

struct Slice {
    explicit Slice(int p, int l = 0) : begin(p), length(l) {}

    int begin;
    int length;
};

Slice identAtCursor(const TextEditor::AssistInterface &interface)
{
    const auto isXidStart = [&interface](const int pos) {
        const QChar c = interface.characterAt(pos);
        return c.isLetter() || c == QLatin1Char('_');
    };

    const auto isXidContinue = [&interface](const int pos) {
        const QChar c = interface.characterAt(pos);
        return c.isLetterOrNumber() || c == QLatin1Char('_');
    };

    int begin = interface.position();
    for (; begin > 0 && isXidContinue(begin - 1); --begin);

    int end = interface.position();
    if (begin == end && isXidStart(end)) {
        ++end;
    }

    for (; isXidContinue(end); ++end);

    return (begin < end) ? Slice(begin, end - begin) : Slice(interface.position());
}

} // namespace

RacerCompletionAssistProvider::RacerCompletionAssistProvider(QObject *parent)
    : CompletionAssistProvider(parent)
{
}

bool RacerCompletionAssistProvider::supportsEditor(Core::Id editorId) const
{
    return editorId == Editors::RUST;
}

TextEditor::IAssistProcessor *RacerCompletionAssistProvider::createProcessor() const
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

TextEditor::IAssistProposal *RacerCompletionAssistProcessor::perform(const TextEditor::AssistInterface *interface)
{
    m_interface.reset(interface);

    QTextCursor cursor(interface->textDocument());
    cursor.setPosition(interface->position());

    if (SourceLayout::isInCommentOrString(cursor)) {
        return nullptr;
    }

    const Slice ident = identAtCursor(*interface);

    if (interface->reason() == TextEditor::IdleEditor &&
            (interface->position() - ident.begin) < MIN_TYPED_CHARS_AUTOCOMPLETE)
        return nullptr;

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

            QList<TextEditor::AssistProposalItemInterface *> proposals;
            for (QRegularExpressionMatchIterator it = match.globalMatch(str); it.hasNext(); ) {
                QRegularExpressionMatch match = it.next();
                if (match.lastCapturedIndex() == 6) {
                    const QString symbol = match.captured(1);
                    const auto type = RacerAssistProposalItem::toType(match.capturedRef(5));
                    const QString detail = match.captured(6);

                    proposals.append(new RacerAssistProposalItem(symbol, detail, type));
                }
            }

            return new TextEditor::GenericProposal(ident.begin, proposals);
        }
    }

    return nullptr;
}

RacerAssistProposalItem::RacerAssistProposalItem(const QString &text,
                                                 const QString &detail,
                                                 RacerAssistProposalItem::Type type)
{
    setText(text);
    setDetail(detail);
    setIcon(iconForType(type));
}

RacerAssistProposalItem::Type RacerAssistProposalItem::toType(QStringRef text)
{
    if (text == QLatin1String("EnumVariant")) {
        return RacerAssistProposalItem::Type::EnumVariant;
    } else if (text == QLatin1String("Function")) {
        return RacerAssistProposalItem::Type::Function;
    } else if (text == QLatin1String("Module")) {
        return RacerAssistProposalItem::Type::Module;
    } else {
        return RacerAssistProposalItem::Type::Other;
    }
}

QIcon RacerAssistProposalItem::iconForType(RacerAssistProposalItem::Type type)
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

} // namespace Internal
} // namespace Rust
