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

#include "FcDoubleSpinBox.h"

#include <QWheelEvent>

#include <FCComponentLib/Components/ComponentMeta.h>
#include <FCComponentLib/Components/ComponentRegistry.h>

namespace FcComponents
{

FcDoubleSpinBox::FcDoubleSpinBox(QWidget* parent)
    : QDoubleSpinBox(parent)
{
    setFocusPolicy(Qt::StrongFocus);
}

bool FcDoubleSpinBox::scrollGuard() const
{
    return m_scrollGuard;
}

void FcDoubleSpinBox::setScrollGuard(bool enabled)
{
    m_scrollGuard = enabled;
}

void FcDoubleSpinBox::wheelEvent(QWheelEvent* event)
{
    if (m_scrollGuard && !hasFocus()) {
        event->ignore();
        return;
    }
    QDoubleSpinBox::wheelEvent(event);
}

}  // namespace FcComponents

FC_REGISTER_COMPONENT(FcDoubleSpinBox, "Inputs", "Double precision spinbox with scroll guard")
