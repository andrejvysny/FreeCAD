// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
 *   Copyright (c) 2025 FreeCAD Project Association                         *
 *                                                                          *
 *   This file is part of FreeCAD.                                          *
 *                                                                          *
 *   FreeCAD is free software: you can redistribute it and/or modify it     *
 *   under the terms of the GNU Lesser General Public License as            *
 *   published by the Free Software Foundation, either version 2.1 of the   *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   FreeCAD is distributed in the hope that it will be useful, but         *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with FreeCAD. If not, see                                *
 *   <https://www.gnu.org/licenses/>.                                       *
 *                                                                          *
 ***************************************************************************/

#include "FcLabelEditor.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QResizeEvent>
#include <QVBoxLayout>

#include <FCComponentLib/Components/ComponentMeta.h>
#include <FCComponentLib/Components/ComponentRegistry.h>
#include <FCComponentLib/Components/Display/FcPropertyListEditor.h>

namespace FcComponents
{

FcLabelEditor::FcLabelEditor(QWidget* parent)
    : QWidget(parent)
{
    m_type = String;
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    m_lineEdit = new QLineEdit(this);
    layout->addWidget(m_lineEdit);

    connect(m_lineEdit, &QLineEdit::textChanged,
            this, &FcLabelEditor::validateText);

    m_button = new QPushButton(QStringLiteral("…"), this);
#if defined(Q_OS_MACOS)
    m_button->setAttribute(Qt::WA_LayoutUsesWidgetRect);
#endif
    layout->addWidget(m_button);

    connect(m_button, &QPushButton::clicked, this, &FcLabelEditor::changeText);

    setFocusProxy(m_lineEdit);
}

FcLabelEditor::~FcLabelEditor() = default;

void FcLabelEditor::resizeEvent(QResizeEvent* event)
{
    m_button->setFixedWidth(event->size().height());
    m_button->setFixedHeight(event->size().height());
}

QString FcLabelEditor::text() const
{
    return m_plainText;
}

void FcLabelEditor::setText(const QString& s)
{
    m_plainText = s;

    QString text = QStringLiteral("[%1]").arg(m_plainText);
    m_lineEdit->setText(text);
}

void FcLabelEditor::changeText()
{
    auto* dlg = new QDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(tr("List"));

    auto* vboxLayout = new QVBoxLayout(dlg);
    auto* buttonBox = new QDialogButtonBox(dlg);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    auto* edit = new FcPropertyListEditor(dlg);
    edit->setPlainText(m_plainText);

    vboxLayout->addWidget(edit);
    vboxLayout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
    connect(dlg, &QDialog::accepted, this, [this, edit] {
        QString inputText = edit->toPlainText();
        QString text = QStringLiteral("[%1]").arg(inputText);
        m_lineEdit->setText(text);
    });

    dlg->exec();
}

void FcLabelEditor::validateText(const QString& text)
{
    if (text.startsWith(QLatin1String("[")) && text.endsWith(QLatin1String("]"))) {
        m_plainText = text.mid(1, text.size() - 2);
        Q_EMIT textChanged(m_plainText);
    }
}

void FcLabelEditor::setButtonText(const QString& txt)
{
    m_button->setText(txt);
    int w1 = 2 * m_button->fontMetrics().horizontalAdvance(txt);
    int w2 = 2 * m_button->fontMetrics().horizontalAdvance(
        QStringLiteral(" … "));
    m_button->setFixedWidth((w1 > w2 ? w1 : w2));
}

QString FcLabelEditor::buttonText() const
{
    return m_button->text();
}

void FcLabelEditor::setInputType(InputType t)
{
    m_type = t;
}

}  // namespace FcComponents

FC_REGISTER_COMPONENT(FcLabelEditor, "Inputs", "Editable label with input types")
