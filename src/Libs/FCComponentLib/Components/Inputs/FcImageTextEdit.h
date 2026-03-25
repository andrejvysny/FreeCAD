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

#include <QTextEdit>

#include <FCComponentLib/FCComponentLibGlobal.h>

class QImage;
class QMimeData;

namespace FcComponents
{

/** @brief Text editor with image drop support.
 *
 *  Extends QTextEdit to allow inserting images via drag-and-drop or
 *  paste from the clipboard.  Dropped images are encoded as inline
 *  base-64 data URIs inside the document.
 */
class FCComponentLibExport FcImageTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    /** @brief Construct an image-capable text editor.
     *  @param parent  Optional parent widget.
     */
    explicit FcImageTextEdit(QWidget* parent = nullptr);

    /** @brief Insert an image into the document at the current cursor position.
     *  @param image   The image to insert.
     *  @param format  Image format string (e.g. "PNG", "JPG", "BMP").
     */
    void dropImage(const QImage& image, const QString& format);

protected:
    /** @brief Check whether the given MIME data can be inserted.
     *
     *  Returns true if the source contains image data or if the base
     *  class accepts it.
     *  @param source  The MIME data to test.
     *  @return True if the data can be inserted.
     */
    bool canInsertFromMimeData(const QMimeData* source) const override;

    /** @brief Insert content from MIME data into the editor.
     *
     *  If the MIME data contains an image, it is inserted as an inline
     *  base-64 data URI.  Otherwise the default QTextEdit behaviour is used.
     *  @param source  The MIME data to insert.
     */
    void insertFromMimeData(const QMimeData* source) override;

    /** @brief Create MIME data from the current selection.
     *  @return A new QMimeData object representing the selection.
     */
    QMimeData* createMimeDataFromSelection() const override;
};

}  // namespace FcComponents
