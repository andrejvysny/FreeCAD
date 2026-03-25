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

#include "FcImageTextEdit.h"

#include <cstdlib>
#include <QBuffer>
#include <QByteArray>
#include <QImage>
#include <QMimeData>
#include <QStringList>
#include <QTextCursor>
#include <QTextImageFormat>

#include <FCComponentLib/Components/ComponentMeta.h>
#include <FCComponentLib/Components/ComponentRegistry.h>

namespace FcComponents
{

FcImageTextEdit::FcImageTextEdit(QWidget* parent)
    : QTextEdit(parent)
{
}

bool FcImageTextEdit::canInsertFromMimeData(const QMimeData* source) const
{
    return source->hasImage() || QTextEdit::canInsertFromMimeData(source);
}

void FcImageTextEdit::insertFromMimeData(const QMimeData* source)
{
    if (source->hasImage()) {
        QStringList formats = source->formats();
        QString format;
        for (int i = 0; i < formats.size(); i++) {
            if (formats[i] == QLatin1String("image/bmp")) {
                format = QString::fromLatin1("BMP");
                break;
            }
            if (formats[i] == QLatin1String("image/jpeg")) {
                format = QString::fromLatin1("JPG");
                break;
            }
            if (formats[i] == QLatin1String("image/jpg")) {
                format = QString::fromLatin1("JPG");
                break;
            }
            if (formats[i] == QLatin1String("image/gif")) {
                format = QString::fromLatin1("GIF");
                break;
            }
            if (formats[i] == QLatin1String("image/png")) {
                format = QString::fromLatin1("PNG");
                break;
            }
            if (formats[i] == QLatin1String("image/pbm")) {
                format = QString::fromLatin1("PBM");
                break;
            }
            if (formats[i] == QLatin1String("image/pgm")) {
                format = QString::fromLatin1("PGM");
                break;
            }
            if (formats[i] == QLatin1String("image/ppm")) {
                format = QString::fromLatin1("PPM");
                break;
            }
            if (formats[i] == QLatin1String("image/tiff")) {
                format = QString::fromLatin1("TIFF");
                break;
            }
            if (formats[i] == QLatin1String("image/xbm")) {
                format = QString::fromLatin1("XBM");
                break;
            }
            if (formats[i] == QLatin1String("image/xpm")) {
                format = QString::fromLatin1("XPM");
                break;
            }
        }
        if (!format.isEmpty()) {
            dropImage(qvariant_cast<QImage>(source->imageData()),
                      QString::fromLatin1("JPG"));
            return;
        }
    }
    QTextEdit::insertFromMimeData(source);
}

QMimeData* FcImageTextEdit::createMimeDataFromSelection() const
{
    return QTextEdit::createMimeDataFromSelection();
}

void FcImageTextEdit::dropImage(const QImage& image, const QString& format)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, format.toLocal8Bit().data());
    buffer.close();
    QByteArray base64 = bytes.toBase64();
    QByteArray base64l;
    for (int i = 0; i < base64.size(); i++) {
        base64l.append(base64[i]);
        if (i % 80 == 0) {
            base64l.append("\n");
        }
    }

    QTextCursor cursor = textCursor();
    QTextImageFormat imageFormat;
    imageFormat.setWidth(image.width());
    imageFormat.setHeight(image.height());
    imageFormat.setName(
        QString::fromLatin1("data:image/%1;base64, %2")
            .arg(QString::fromLatin1("%1.%2").arg(rand()).arg(format))
            .arg(QString::fromLatin1(base64l.data())));
    cursor.insertImage(imageFormat);
}

}  // namespace FcComponents

FC_REGISTER_COMPONENT(FcImageTextEdit,
                       "Inputs",
                       "Text editor with image drop support")
