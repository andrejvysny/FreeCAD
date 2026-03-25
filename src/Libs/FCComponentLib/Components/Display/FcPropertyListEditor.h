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

#include <QPlainTextEdit>

#include <FCComponentLib/FCComponentLibGlobal.h>

namespace FcComponents
{

/** @brief A plain text editor with a line number area.
 *
 * Provides a QPlainTextEdit with an integrated line number gutter on the
 * left side. The current line is highlighted and the line number area
 * automatically adjusts its width based on the total number of lines.
 */
class FCComponentLibExport FcPropertyListEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    /** @brief Constructs a property list editor widget.
     *  @param parent The parent widget.
     */
    explicit FcPropertyListEditor(QWidget* parent = nullptr);

    /** @brief Paints line numbers in the line number area.
     *
     *  Called by the internal line number area widget's paintEvent.
     *  @param event The paint event to handle.
     */
    void lineNumberAreaPaintEvent(QPaintEvent* event);

    /** @brief Calculates the required width for the line number area.
     *  @return The width in pixels needed to display all line numbers.
     */
    int lineNumberAreaWidth();

protected:
    /** @brief Handles resize events to reposition the line number area.
     *  @param event The resize event.
     */
    void resizeEvent(QResizeEvent* event) override;

private Q_SLOTS:
    /** @brief Updates the viewport margin when the block count changes.
     *  @param newBlockCount The new number of text blocks.
     */
    void updateLineNumberAreaWidth(int newBlockCount);

    /** @brief Highlights the current line with a selection color. */
    void highlightCurrentLine();

    /** @brief Scrolls or repaints the line number area on text updates.
     *  @param rect The region that was updated.
     *  @param dy The vertical scroll offset.
     */
    void updateLineNumberArea(const QRect& rect, int dy);

private:
    QWidget* m_lineNumberArea;
};

}  // namespace FcComponents
