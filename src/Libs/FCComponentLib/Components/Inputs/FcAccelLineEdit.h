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

#include <QKeySequenceEdit>

#include <FCComponentLib/FCComponentLibGlobal.h>

namespace FcComponents
{

/** @brief A keyboard shortcut input field based on QKeySequenceEdit.
 *
 * Captures key sequences for use as keyboard shortcuts. Includes a clear
 * button and provides convenience methods for read-only mode, emptiness
 * checks, and text representation.
 */
class FCComponentLibExport FcAccelLineEdit : public QKeySequenceEdit
{
    Q_OBJECT

public:
    /** @brief Constructs an accelerator line edit.
     *  @param parent The parent widget.
     */
    explicit FcAccelLineEdit(QWidget* parent = nullptr);

    /** @brief Constructs an accelerator line edit with an initial key sequence.
     *  @param keySequence The initial key sequence.
     *  @param parent The parent widget.
     */
    explicit FcAccelLineEdit(const QKeySequence& keySequence,
                             QWidget* parent = nullptr);

    /** @brief Sets the read-only state of the internal line edit.
     *  @param value True to make read-only.
     */
    void setReadOnly(bool value);

    /** @brief Returns true if no key sequence is set. */
    bool isEmpty() const;

    /** @brief Returns the key sequence as a human-readable string. */
    QString text() const;
};

}  // namespace FcComponents
