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
#include "rustgrammar.h"
#include "rustlexer.h"
#include "rustsourcelayout.h"
#include "rusttoken.h"

#include <coreplugin/id.h>
#include <texteditor/codeassist/assistinterface.h>
#include <texteditor/codeassist/assistproposalitem.h>
#include <texteditor/codeassist/functionhintproposal.h>
#include <texteditor/codeassist/genericproposal.h>
#include <texteditor/texteditor.h>
#include <utils/algorithm.h>

#include <QProcess>
#include <QRegularExpression>
#include <QStringList>
#include <QTemporaryFile>
#include <QTextBlock>
#include <QTextStream>
#include <QVector>

#include <algorithm>

namespace Rust {
namespace Internal {
namespace {

constexpr int RACER_TIMEOUT_MSEC = 5000;
constexpr int MIN_TYPED_CHARS_AUTOCOMPLETE = 3;

struct Slice {
    Slice() : begin(-1), length(0) {}
    explicit Slice(int p, int l = 0) : begin(p), length(l) {}

    int begin;
    int length;
};

Slice identAtCursor(const TextEditor::AssistInterface &interface)
{
    const auto isXidStart = [&interface](const int pos) {
        const QChar c = interface.characterAt(pos);
        return Grammar::isXidStart(c);
    };

    const auto isXidContinue = [&interface](const int pos) {
        const QChar c = interface.characterAt(pos);
        return Grammar::isXidContinue(c);
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

void appendKeywords(QList<TextEditor::AssistProposalItemInterface *>& proposals)
{
    const auto addProposal = [&proposals](const QLatin1String& text) {
        proposals.append(new RacerAssistProposalItem(Racer::Result::Type::Keyword, text));
    };

    std::for_each(KEYWORDS.begin(), KEYWORDS.end(), addProposal);
    std::for_each(INT_TYPES.begin(), INT_TYPES.end(), addProposal);
    std::for_each(FLOAT_TYPES.begin(), FLOAT_TYPES.end(), addProposal);
    std::for_each(OTHER_PRIMITIVE_TYPES.begin(), OTHER_PRIMITIVE_TYPES.end(), addProposal);
}

template<typename F>
void forEachFunArg(QStringRef declaration, F fn)
{
    Lexer lexer(declaration);

    bool self = false;
    int depth = 0;
    Slice slice;
    while (const Token token = lexer.next()) {
        if (token.type == TokenType::ParenthesisLeft) {
            if (depth == 0) {
                slice.begin = token.begin + 1;
            }
            ++depth;
        } else if (token.type == TokenType::ParenthesisRight) {
            if (depth == 0) {
                break;
            }

            --depth;

            if (depth == 0) {
                if (slice.begin >= 0) {
                    slice.length = token.begin - slice.begin;
                    if (!self) {
                        fn(slice);
                    } else {
                        self = false;
                    }
                    slice = Slice();
                }
                break;
            }
        } else if (token.type == TokenType::Comma && depth == 1) {
            if (slice.begin >= 0) {
                slice.length = token.begin - slice.begin;
                if (!self) {
                    fn(slice);
                } else {
                    self = false;
                }
                slice.begin = token.begin + 1;
                slice.length = 0;
            } else {
                break;
            }
        } else if (token.type == TokenType::Keyword && depth == 1 &&
                   declaration.mid(token.begin, token.length) == KEYWORD_LC_SELF) {
            self = true;
        }
    }
}

} // namespace

namespace Racer {

QVector<Result> run(Request request,
                    const QTextCursor& cursor,
                    const TextEditor::AssistInterface &interface)
{
    QVector<Result> results;

    const QTextBlock block = cursor.block();
    const int line = block.blockNumber() + 1;
    int column = cursor.positionInBlock();

    QTemporaryFile file;
    if (file.open()) {
        {
            QTextStream out(&file);
            out.setCodec("UTF-8");
            out << interface.textDocument()->toPlainText();
        }

        const QString program = QLatin1String("racer");
        const QStringList arguments = {
            QLatin1String("complete"),
            QString::number(line),
            QString::number(column),
            interface.fileName(),
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
    return (sequence.endsWith(CHAR_POINT) && sequence != RANGE_OPERATOR) ||
            sequence == PATH_SEPARATOR ||
            sequence.endsWith(CHAR_PARENTHESES_LEFT);
}

TextEditor::IAssistProposal *RacerCompletionAssistProcessor::perform(const TextEditor::AssistInterface *interface)
{
    m_interface.reset(interface);

    QTextCursor cursor(interface->textDocument());
    cursor.setPosition(interface->position());

    if (SourceLayout::isInCommentOrString(cursor)) {
        return nullptr;
    }

    if (interface->reason() != TextEditor::IdleEditor && interface->position() > 0 &&
            interface->characterAt(interface->position()-1) == QLatin1Char('(')) {
        int pos = interface->position() - 2;
        for(; pos >= 0 && interface->characterAt(pos).isSpace(); --pos);
        if (pos >= 0 && Grammar::isXidContinue(interface->characterAt(pos))) {
            QTextCursor newCursor(cursor);
            newCursor.setPosition(pos);
            auto results = Racer::run(Racer::Request::FindDefinition, newCursor, *interface);

            QStringList declarations;
            for (const Racer::Result& result : results) {
                if (result.type == Racer::Result::Type::Function) {
                    declarations.push_back(result.detail);
                }
            }

            if (!declarations.isEmpty()) {
                auto model = new RacerFunctionHintProposalModel(declarations);
                return new TextEditor::FunctionHintProposal(interface->position()-1, model);
            } else {
                return nullptr;
            }
        } else if (interface->reason() == TextEditor::ActivationCharacter) {
            return nullptr;
        }
    }

    const Slice ident = identAtCursor(*interface);

    if (interface->reason() == TextEditor::IdleEditor &&
            (interface->position() - ident.begin) < MIN_TYPED_CHARS_AUTOCOMPLETE) {
        return nullptr;
    }

    QList<TextEditor::AssistProposalItemInterface *> proposals;
    for (const Racer::Result& result : Racer::run(Racer::Request::Complete, cursor, *interface)) {
        proposals.push_back(new RacerAssistProposalItem(result));
    }

    if (interface->reason() == TextEditor::IdleEditor) {
        appendKeywords(proposals);
    }

    return new TextEditor::GenericProposal(ident.begin, proposals);
}

RacerAssistProposalItem::RacerAssistProposalItem(const Racer::Result& result)
{
    setText(result.symbol);
    setDetail(result.detail);
    setIcon(Racer::Result::icon(result.type));
}

RacerAssistProposalItem::RacerAssistProposalItem(Racer::Result::Type type,
                                                 const QString& symbol,
                                                 const QString& detail)
{
    setText(symbol);
    setDetail(detail);
    setIcon(Racer::Result::icon(type));
}

RacerFunctionHintProposalModel::RacerFunctionHintProposalModel(QStringList declarations)
    : m_declarations(declarations),
      m_currentArg(-1)
{
}

void RacerFunctionHintProposalModel::reset()
{
    m_currentArg = -1;
}

int RacerFunctionHintProposalModel::size() const
{
    return m_declarations.size();
}

QString RacerFunctionHintProposalModel::text(int index) const
{
    const QString& declaration = m_declarations.at(index);

    if (m_currentArg >= 0) {
        int count = 0;
        Slice arg;
        forEachFunArg(&declaration, [&count, &arg, this](const Slice& slice){
            if (count == m_currentArg) {
                arg = slice;
            }
            ++count;
        });

        if (arg.begin >= 0) {
            QString hintText;
            hintText += declaration.left(arg.begin).toHtmlEscaped();
            hintText += QLatin1String("<b>");
            hintText += declaration.mid(arg.begin, arg.length).toHtmlEscaped();
            hintText += QLatin1String("</b>");
            hintText += declaration.mid(arg.begin + arg.length).toHtmlEscaped();
            return hintText;
        }
    }

    return declaration.toHtmlEscaped();
}

int RacerFunctionHintProposalModel::activeArgument(const QString &prefix) const
{
    m_currentArg = 0;
    forEachFunArg(&prefix, [this](const Slice& slice){ ++m_currentArg; });
    return m_currentArg;
}

} // namespace Internal
} // namespace Rust
