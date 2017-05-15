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

#pragma once

#include <texteditor/codeassist/completionassistprovider.h>
#include <texteditor/codeassist/keywordscompletionassist.h>
#include <texteditor/codeassist/assistproposalitem.h>

namespace Rust {
namespace Internal {

class RacerCompletionAssistProcessor : public TextEditor::KeywordsCompletionAssistProcessor
{
public:
    RacerCompletionAssistProcessor();
    ~RacerCompletionAssistProcessor();

    TextEditor::IAssistProposal *perform(const TextEditor::AssistInterface *interface) override;
};

class RacerCompletionAssistProvider : public TextEditor::CompletionAssistProvider
{
    Q_OBJECT

public:
    RacerCompletionAssistProvider(QObject *parent = nullptr);

    bool supportsEditor(Core::Id editorId) const override;
    TextEditor::IAssistProcessor *createProcessor() const override;
    int activationCharSequenceLength() const override;
    bool isActivationCharSequence(const QString &sequence) const override;
};

class RacerAssistProposalItem : public TextEditor::AssistProposalItem
{
public:
    enum class Type {
        Function,
        Other
    };

    RacerAssistProposalItem(const QString &text, const QString &detail, Type type);

    static QIcon iconForType(Type type);
};

} // namespace Internal
} // namespace Rust
