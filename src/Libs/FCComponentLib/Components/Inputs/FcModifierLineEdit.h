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

#include <QLineEdit>

#include <FCComponentLib/FCComponentLibGlobal.h>

namespace FcComponents
{

/** @brief A line edit that captures keyboard modifier keys (Ctrl, Shift, Alt, Meta).
 *
 * Displays the currently pressed modifier combination as text. Non-modifier
 * keys are ignored. Backspace and Delete clear the field.
 */
class FCComponentLibExport FcModifierLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    /** @brief Constructs a modifier line edit.
     *  @param parent The parent widget.
     */
    explicit FcModifierLineEdit(QWidget* parent = nullptr);

protected:
    /** @brief Captures modifier key presses and displays them as text.
     *  @param event The key press event.
     */
    void keyPressEvent(QKeyEvent* event) override;
};

}  // namespace FcComponents
