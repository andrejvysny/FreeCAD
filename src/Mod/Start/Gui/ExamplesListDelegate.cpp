// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
 *   Copyright (c) 2024 The FreeCAD Project Association AISBL               *
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

#include <QApplication>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QModelIndex>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionViewItem>

#include "../App/DisplayedFilesModel.h"
#include "ExamplesListDelegate.h"

using namespace StartGui;

ExamplesListDelegate::ExamplesListDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{}

QSize ExamplesListDelegate::sizeHint(
    const QStyleOptionViewItem& option,
    const QModelIndex& index
) const
{
    Q_UNUSED(index);
    return {option.rect.width(), rowHeight};
}

void ExamplesListDelegate::paint(
    QPainter* painter,
    const QStyleOptionViewItem& option,
    const QModelIndex& index
) const
{
    painter->save();

    // Step 1: Draw hover/selection background
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    const QWidget* widget = option.widget;
    QStyle* style = widget ? widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, widget);

    // Step 2: Fetch thumbnail
    auto imageData = index.data(static_cast<int>(Start::DisplayedFilesModelRoles::image)).toByteArray();
    auto path = index.data(static_cast<int>(Start::DisplayedFilesModelRoles::path)).toString();

    QPixmap pixmap;
    if (!imageData.isEmpty()) {
        pixmap.loadFromData(imageData);
    }
    if (pixmap.isNull()) {
        QIcon icon = QFileIconProvider().icon(QFileInfo(path));
        if (!icon.isNull()) {
            pixmap = icon.pixmap(thumbnailSize, thumbnailSize);
        }
    }

    QPixmap scaledPixmap;
    if (!pixmap.isNull()) {
        scaledPixmap = pixmap.scaled(
            QSize(thumbnailSize, thumbnailSize),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );
    }

    // Step 3: Layout geometry
    int iconLeft = option.rect.x() + margin;
    int iconTop = option.rect.y() + (rowHeight - thumbnailSize) / 2;
    QRect iconRect(iconLeft, iconTop, thumbnailSize, thumbnailSize);

    int textLeft = iconRect.right() + spacing;
    int textWidth = option.rect.right() - textLeft - margin;
    QFontMetrics fm = painter->fontMetrics();

    int totalTextHeight = fm.lineSpacing() * 2 + spacing;
    int textTop = option.rect.y() + (rowHeight - totalTextHeight) / 2;

    QRect nameRect(textLeft, textTop, textWidth, fm.lineSpacing());
    QRect sizeRect(textLeft, nameRect.bottom() + spacing, textWidth, fm.lineSpacing());

    // Step 4: Draw thumbnail
    if (!scaledPixmap.isNull()) {
        QRect drawRect(iconRect.topLeft(), scaledPixmap.size());
        drawRect.moveCenter(iconRect.center());
        painter->drawPixmap(drawRect.topLeft(), scaledPixmap);
    }

    // Step 5: Draw filename
    auto baseName = index.data(static_cast<int>(Start::DisplayedFilesModelRoles::baseName)).toString();
    QString elidedName = fm.elidedText(baseName, Qt::ElideRight, textWidth);
    painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, elidedName);

    // Step 6: Draw file size in lighter color
    auto fileSize = index.data(static_cast<int>(Start::DisplayedFilesModelRoles::size)).toString();
    QColor lightColor = option.palette.color(QPalette::Text);
    lightColor.setAlphaF(0.55);
    painter->setPen(lightColor);
    painter->drawText(sizeRect, Qt::AlignLeft | Qt::AlignVCenter, fileSize);

    painter->restore();
}
