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

#include <QComboBox>

#include <FCComponentLib/FCComponentLibGlobal.h>

namespace FcComponents
{

/// Combo box with scroll guard to prevent accidental selection changes
/// when the widget is inside a scrollable panel.
class FCComponentLibExport FcComboBox : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY(bool scrollGuard READ scrollGuard WRITE setScrollGuard)

public:
    explicit FcComboBox(QWidget* parent = nullptr);

    bool scrollGuard() const;
    void setScrollGuard(bool enabled);

protected:
    void wheelEvent(QWheelEvent* event) override;

private:
    bool m_scrollGuard = true;
};

}  // namespace FcComponents
