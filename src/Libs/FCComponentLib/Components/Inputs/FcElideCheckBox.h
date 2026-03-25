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

#include <QCheckBox>

#include <FCComponentLib/FCComponentLibGlobal.h>

namespace FcComponents
{

/** @brief Checkbox that elides its label text with an ellipsis when space is limited.
 *
 * This widget extends QCheckBox to add text ellipsis functionality,
 * preventing long labels from overflowing their available space.
 */
class FCComponentLibExport FcElideCheckBox : public QCheckBox
{
    Q_OBJECT

public:
    /** @brief Construct an elide checkbox. */
    explicit FcElideCheckBox(QWidget* parent = nullptr);
    ~FcElideCheckBox() override = default;

protected:
    /** @brief Paint the checkbox with elided text when the label exceeds available width. */
    void paintEvent(QPaintEvent* event) override;

    /** @brief Return the ideal size based on the full (non-elided) text width. */
    QSize sizeHint() const override;

    /** @brief Return the minimum size allowing a short elided placeholder. */
    QSize minimumSizeHint() const override;
};

}  // namespace FcComponents
