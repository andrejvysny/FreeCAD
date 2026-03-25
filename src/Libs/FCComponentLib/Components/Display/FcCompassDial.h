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

#include <QSize>
#include <QWidget>

#include <FCComponentLib/FCComponentLibGlobal.h>

namespace FcComponents
{

/** @brief Compass rose dial widget with a rotatable needle.
 *
 *  Displays a compass-style dial with cardinal direction markings (X/Y axes)
 *  and a needle that can be rotated to any angle. Based on a Python widget from
 *  https://github.com/tcalmant/demo-ipopo-qt/blob/master/pc/details/compass.py
 */
class FCComponentLibExport FcCompassDial : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double angle READ angle WRITE setAngle)

public:
    /** @brief Construct a compass dial widget. */
    explicit FcCompassDial(QWidget* parent = nullptr);
    ~FcCompassDial() override = default;

    /** @brief Return the recommended size for the widget. */
    QSize sizeHint() const override;

    /** @brief Return the minimum recommended size for the widget. */
    QSize minimumSizeHint() const override;

    /** @brief Return the current needle angle in degrees. */
    double angle() const
    {
        return m_angle;
    }

    /** @brief Set the needle angle in conventional degrees (converted for Qt painting). */
    void setAngle(double newAngle);

    /** @brief Set the overall size of the dial widget. */
    void setSize(int newSize);

public Q_SLOTS:
    /** @brief Slot to change the needle angle. */
    void slotChangeAngle(double angle)
    {
        setAngle(angle);
    }

    /** @brief Slot to reset the needle angle to zero. */
    void resetAngle()
    {
        setAngle(0.0);
    }

protected:
    void paintEvent(QPaintEvent* event) override;
    void drawWidget(QPainter& painter);
    void drawNeedle(QPainter& painter);
    void drawMarkings(QPainter& painter);
    void drawBackground(QPainter& painter);

private:
    QRect m_rect;
    double m_angle;
    double m_margin;
    double m_markInterval;
    int m_defaultSize;
    int m_defaultMargin;
    int m_designRadius;
    int m_designDiameter;
};

}  // namespace FcComponents
