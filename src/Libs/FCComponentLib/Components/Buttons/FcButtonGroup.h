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

#include <QButtonGroup>

#include <FCComponentLib/FCComponentLibGlobal.h>

namespace FcComponents
{

/** @brief A button group that allows unchecking all buttons in exclusive mode.
 *
 * Unlike Qt's QButtonGroup, this class implements exclusive mode manually so
 * that clicking an already-checked button will uncheck it, leaving no button
 * selected. QButtonGroup's built-in exclusive mode prevents this.
 */
class FCComponentLibExport FcButtonGroup : public QButtonGroup
{
    Q_OBJECT

public:
    /** @brief Constructs a button group.
     *  @param parent The parent object.
     */
    explicit FcButtonGroup(QObject* parent = nullptr);

    /** @brief Sets whether the group operates in exclusive mode.
     *  @param on True for exclusive mode (at most one button checked).
     */
    void setExclusive(bool on);

    /** @brief Returns whether the group operates in exclusive mode. */
    bool exclusive() const;

private:
    bool m_exclusive;
};

}  // namespace FcComponents
