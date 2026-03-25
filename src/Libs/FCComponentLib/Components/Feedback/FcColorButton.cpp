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

#include "FcColorButton.h"

#include <QColorDialog>
#include <QPainter>
#include <QStyleOptionButton>
#include <QStylePainter>

#include <FCComponentLib/Components/ComponentMeta.h>
#include <FCComponentLib/Components/ComponentRegistry.h>

namespace FcComponents
{

FcColorButton::FcColorButton(QWidget* parent)
    : QPushButton(parent)
{
    setMinimumSize(40, 24);
    connect(this, &QPushButton::clicked, this, &FcColorButton::onClicked);
}

QColor FcColorButton::color() const
{
    return m_color;
}

void FcColorButton::setColor(const QColor& color)
{
    if (m_color == color) {
        return;
    }
    m_color = color;
    update();
    Q_EMIT colorChanged(m_color);
}

bool FcColorButton::allowTransparency() const
{
    return m_allowTransparency;
}

void FcColorButton::setAllowTransparency(bool allow)
{
    m_allowTransparency = allow;
}

void FcColorButton::paintEvent(QPaintEvent* event)
{
    QPushButton::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw a color swatch inset from the button edges
    constexpr int margin = 5;
    QRect colorRect = rect().adjusted(margin, margin, -margin, -margin);

    // Draw a checkerboard background to visualize transparency
    if (m_color.alpha() < 255) {
        constexpr int gridSize = 4;
        for (int y = colorRect.top(); y < colorRect.bottom(); y += gridSize) {
            for (int x = colorRect.left(); x < colorRect.right(); x += gridSize) {
                bool light = ((x - colorRect.left()) / gridSize
                              + (y - colorRect.top()) / gridSize)
                        % 2
                    == 0;
                painter.fillRect(QRect(x, y, gridSize, gridSize),
                                 light ? Qt::white : QColor(204, 204, 204));
            }
        }
    }

    painter.fillRect(colorRect, m_color);
    painter.setPen(QPen(palette().mid().color(), 1));
    painter.drawRect(colorRect);
}

void FcColorButton::onClicked()
{
    QColorDialog::ColorDialogOptions options;
    if (m_allowTransparency) {
        options |= QColorDialog::ShowAlphaChannel;
    }

    QColor selected = QColorDialog::getColor(m_color, this, tr("Select Color"), options);
    if (selected.isValid()) {
        setColor(selected);
    }
}

}  // namespace FcComponents

FC_REGISTER_COMPONENT(FcColorButton, "Feedback", "Color picker button with preview")
