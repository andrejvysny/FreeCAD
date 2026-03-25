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

#include "FcElideCheckBox.h"

#include <QFontMetrics>
#include <QPainter>
#include <QStyleOptionButton>

#include <FCComponentLib/Components/ComponentMeta.h>
#include <FCComponentLib/Components/ComponentRegistry.h>

namespace FcComponents
{

FcElideCheckBox::FcElideCheckBox(QWidget* parent)
    : QCheckBox(parent)
{}

void FcElideCheckBox::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QStyleOptionButton option;
    option.initFrom(this);

    option.state = (isChecked() ? QStyle::State_On : QStyle::State_Off)
        | (isEnabled() ? QStyle::State_Enabled : QStyle::State_ReadOnly)
        | (underMouse() ? QStyle::State_MouseOver : QStyle::State_None)
        | (hasFocus() ? QStyle::State_HasFocus : QStyle::State_None);

    QPainter painter(this);

    style()->drawControl(QStyle::CE_CheckBox, &option, &painter, this);

    QRect textRect = style()->subElementRect(QStyle::SE_CheckBoxContents, &option, this);

    QFontMetrics fm(font());
    QString elidedText = fm.elidedText(text(), Qt::ElideRight, textRect.width());

    painter.setPen(palette().color(QPalette::WindowText));
    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, elidedText);
}

QSize FcElideCheckBox::sizeHint() const
{
    QFontMetrics fm(font());
    int width = fm.horizontalAdvance(this->text())
        + style()->pixelMetric(QStyle::PM_IndicatorWidth, nullptr, this)
        + style()->pixelMetric(QStyle::PM_CheckBoxLabelSpacing, nullptr, this);
    int height = fm.height();
    return {width, height};
}

QSize FcElideCheckBox::minimumSizeHint() const
{
    QFontMetrics fm(font());
    QString minimumText = QStringLiteral("A...");
    int width = fm.horizontalAdvance(minimumText)
        + style()->pixelMetric(QStyle::PM_IndicatorWidth, nullptr, this)
        + style()->pixelMetric(QStyle::PM_CheckBoxLabelSpacing, nullptr, this);
    int height = fm.height();
    return {width, height};
}

}  // namespace FcComponents

FC_REGISTER_COMPONENT(FcElideCheckBox, "Inputs", "Checkbox with text ellipsis")
