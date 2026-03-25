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
#include <QString>

#include <FCComponentLib/FCComponentLibGlobal.h>

namespace FcComponents
{

class ColorPickerPopup;

/** @brief Color picker button with a popup color grid.
 *
 *  Users can click the button (or press Space) to open a popup grid of
 *  colors.  Navigation is possible with the mouse or arrow keys.
 *  Selecting a color emits colorChanged().
 *
 *  An optional "..." (more) button at the bottom of the grid opens a
 *  full QColorDialog so the user can pick any color.
 *
 *  Colors are added with insertColor() or in bulk with setStandardColors().
 *  The static helper getColor() shows a one-shot popup at an arbitrary
 *  screen position.
 */
class FCComponentLibExport FcColorPicker : public QPushButton
{
    Q_OBJECT

    Q_PROPERTY(bool colorDialog READ colorDialogEnabled WRITE setColorDialogEnabled)

public:
    /** @brief Construct a color picker button.
     *
     *  @param parent            Parent widget.
     *  @param columns           Number of columns in the popup grid (-1 = auto).
     *  @param enableColorDialog Whether the "More" button is shown.
     */
    explicit FcColorPicker(QWidget* parent = nullptr,
                           int columns = -1,
                           bool enableColorDialog = true);

    ~FcColorPicker() override;

    /** @brief Add a color to the popup grid.
     *
     *  @param color  The color to insert.
     *  @param text   Display name for the color.
     *  @param index  Position (-1 appends).
     */
    void insertColor(const QColor& color,
                     const QString& text = QString(),
                     int index = -1);

    /** @brief Return the currently selected color. */
    QColor currentColor() const;

    /** @brief Return the color at the given grid index. */
    QColor color(int index) const;

    /** @brief Enable or disable the "More" color-dialog button. */
    void setColorDialogEnabled(bool enabled);

    /** @brief Return whether the "More" button is enabled. */
    bool colorDialogEnabled() const;

    /** @brief Populate the grid with the 17 standard Qt colors. */
    void setStandardColors();

    /** @brief Show a one-shot color grid popup at the given position.
     *
     *  @param pos               Global screen position.
     *  @param allowCustomColors Whether the "More" button is shown.
     *  @return The color chosen by the user.
     */
    static QColor getColor(const QPoint& pos, bool allowCustomColors = true);

public Q_SLOTS:
    /** @brief Set the current color, inserting it if not already present. */
    void setCurrentColor(const QColor& col);

Q_SIGNALS:
    /** @brief Emitted when the selected color changes. */
    void colorChanged(const QColor&);

    /** @brief Emitted when a color is confirmed (even if unchanged). */
    void colorSet(const QColor&);

protected:
    /** @brief Repaints the color-swatch icon when dirty. */
    void paintEvent(QPaintEvent* e) override;

private Q_SLOTS:
    void buttonPressed(bool toggled);
    void popupClosed();

private:
    ColorPickerPopup* m_popup;
    QColor m_color;
    bool m_withColorDialog;
    bool m_dirty;
    bool m_firstInserted;
};

}  // namespace FcComponents
