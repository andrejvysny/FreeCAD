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

class QAction;

namespace FcComponents
{

/** @brief A line edit with an embedded clear button on the right side.
 *
 * The clear button appears as a trailing action and is only visible when
 * the line edit contains text. Clicking it clears the field.
 */
class FCComponentLibExport FcClearLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    /** @brief Constructs a clear line edit.
     *  @param parent The parent widget.
     */
    explicit FcClearLineEdit(QWidget* parent = nullptr);

protected:
    /** @brief Handles resize events.
     *  @param event The resize event.
     */
    void resizeEvent(QResizeEvent* event) override;

private Q_SLOTS:
    /** @brief Shows or hides the clear button based on text content.
     *  @param text The current text in the line edit.
     */
    void updateClearButton(const QString& text);

private:
    QAction* m_clearAction;
};

}  // namespace FcComponents
