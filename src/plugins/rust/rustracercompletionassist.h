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

#include "rustracer.h"

#include <texteditor/codeassist/completionassistprovider.h>
#include <texteditor/codeassist/assistproposalitem.h>
#include <texteditor/codeassist/iassistprocessor.h>
#include <texteditor/codeassist/ifunctionhintproposalmodel.h>

#include <QScopedPointer>

namespace Rust {
namespace Internal {

class RacerCompletionAssistProvider final : public TextEditor::CompletionAssistProvider
{
    Q_OBJECT

public:
    RacerCompletionAssistProvider(QObject *parent = nullptr);

    bool supportsEditor(Core::Id editorId) const override;
    TextEditor::IAssistProcessor *createProcessor() const override;
    int activationCharSequenceLength() const override;
    bool isActivationCharSequence(const QString &sequence) const override;
};

class RacerCompletionAssistProcessor final : public TextEditor::IAssistProcessor
{
public:
    TextEditor::IAssistProposal *perform(const TextEditor::AssistInterface *interface) override;

private:
    QScopedPointer<const TextEditor::AssistInterface> m_interface;
};

class RacerAssistProposalItem final : public TextEditor::AssistProposalItem
{
public:
    explicit RacerAssistProposalItem(const Racer::Result& result);
    RacerAssistProposalItem(Racer::Result::Type type, const QString& symbol, const QString& detail = QString());
};

class RacerFunctionHintProposalModel final : public TextEditor::IFunctionHintProposalModel
{
public:
    explicit RacerFunctionHintProposalModel(QStringList declarations);

    void reset() override;
    int size() const override;
    QString text(int index) const override;
    int activeArgument(const QString &prefix) const override;

private:
    QStringList m_declarations;
    mutable int m_currentArg;
};

} // namespace Internal
} // namespace Rust
