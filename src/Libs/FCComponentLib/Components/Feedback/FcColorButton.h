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

#pragma once

#include <QColor>
#include <QPushButton>

#include <FCComponentLib/FCComponentLibGlobal.h>

namespace FcComponents
{

/// Color picker button that displays the current color and opens a
/// QColorDialog when clicked.
class FCComponentLibExport FcColorButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(bool allowTransparency READ allowTransparency WRITE setAllowTransparency)

public:
    explicit FcColorButton(QWidget* parent = nullptr);

    QColor color() const;
    void setColor(const QColor& color);

    bool allowTransparency() const;
    void setAllowTransparency(bool allow);

Q_SIGNALS:
    void colorChanged(const QColor& color);

protected:
    void paintEvent(QPaintEvent* event) override;

private Q_SLOTS:
    void onClicked();

private:
    QColor m_color {Qt::white};
    bool m_allowTransparency = false;
};

}  // namespace FcComponents
