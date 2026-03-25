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

#include "FcLabelButton.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>

#include <FCComponentLib/Components/ComponentMeta.h>
#include <FCComponentLib/Components/ComponentRegistry.h>

namespace FcComponents
{

FcLabelButton::FcLabelButton(QWidget* parent)
    : QWidget(parent)
{
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(1);

    m_label = new QLabel(this);
    m_label->setAutoFillBackground(true);
    layout->addWidget(m_label);

    m_button = new QPushButton(QStringLiteral("…"), this);
#if defined(Q_OS_MACOS)
    m_button->setAttribute(Qt::WA_LayoutUsesWidgetRect);
#endif
    layout->addWidget(m_button);

    connect(m_button, &QPushButton::clicked, this, &FcLabelButton::browse);
    connect(m_button, &QPushButton::clicked, this, &FcLabelButton::buttonClicked);
}

FcLabelButton::~FcLabelButton() = default;

void FcLabelButton::resizeEvent(QResizeEvent* event)
{
    m_button->setFixedWidth(event->size().height());
    m_button->setFixedHeight(event->size().height());
}

QLabel* FcLabelButton::getLabel() const
{
    return m_label;
}

QPushButton* FcLabelButton::getButton() const
{
    return m_button;
}

QVariant FcLabelButton::value() const
{
    return m_value;
}

void FcLabelButton::setValue(const QVariant& val)
{
    m_value = val;
    showValue(m_value);
    Q_EMIT valueChanged(m_value);
}

void FcLabelButton::showValue(const QVariant& data)
{
    m_label->setText(data.toString());
}

void FcLabelButton::browse()
{}

}  // namespace FcComponents

FC_REGISTER_COMPONENT(FcLabelButton, "Inputs", "Label with action button")
