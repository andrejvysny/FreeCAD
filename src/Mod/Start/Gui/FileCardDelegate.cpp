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

#include <QFile>
#include <QFileIconProvider>
#include <QImageReader>
#include <QPainter>
#include <QPen>
#include <QStyleOptionViewItem>
#include <QLabel>
#include <QModelIndex>
#include <QVBoxLayout>
#include <QApplication>
#include <QString>
#include <QAbstractItemView>

#include "FileCardDelegate.h"
#include "../App/DisplayedFilesModel.h"
#include "../App/FileUtilities.h"
#include "App/Application.h"
#include <Base/Color.h>
#include <Base/Console.h>
#include <Gui/Application.h>
#include <Gui/MainWindow.h>

using namespace Start;

QCache<QString, QPixmap> FileCardDelegate::_thumbnailCache;

FileCardDelegate::FileCardDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
    _parameterGroup = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Mod/Start"
    );
    setObjectName(QStringLiteral("thumbnailWidget"));

    // Initialize cache size based on thumbnail size (only once)
    if (_thumbnailCache.maxCost() == 0) {
        int thumbnailSize = static_cast<int>(_parameterGroup->GetInt("FileThumbnailIconsSize", 128));
        int thumbnailMemory = thumbnailSize * thumbnailSize * 4;  // rgba
        int maxCacheItems = (CACHE_SIZE_MB * 1024 * 1024) / thumbnailMemory;
        _thumbnailCache.setMaxCost(maxCacheItems);
        Base::Console().log(
            "FileCardDelegate: Initialized thumbnail cache for %d items (%d MB)\n",
            maxCacheItems,
            CACHE_SIZE_MB
        );
    }
}

bool FileCardDelegate::isDarkTheme() const
{
    auto mainWindowGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/MainWindow");
    auto theme = QString::fromStdString(mainWindowGrp->GetASCII("Theme", "Classic"));
    if (theme.contains(QLatin1String("Dark"), Qt::CaseInsensitive)) {
        return true;
    }
    if (theme.contains(QLatin1String("Light"), Qt::CaseInsensitive)) {
        return false;
    }
    // Fallback for "Classic" or unknown themes
    QColor windowBg = qApp->palette().window().color();
    double luminance = 0.299 * windowBg.redF() + 0.587 * windowBg.greenF() + 0.114 * windowBg.blueF();
    return luminance < 0.5;
}

void FileCardDelegate::paintOpenFileCard(
    QPainter* painter,
    const QStyleOptionViewItem& option
) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    auto thumbnailSize = static_cast<int>(_parameterGroup->GetInt("FileThumbnailIconsSize", 128));
    int spacing = static_cast<int>(_parameterGroup->GetInt("FileCardSpacing", 16));
    QRect cardRect = option.rect.adjusted(0, 0, -spacing, -spacing);

    bool isDark = isDarkTheme();

    // Card background fill
    QColor cardBg = isDark ? QColor(0x38, 0x38, 0x38) : QColor(Qt::white);
    painter->setBrush(cardBg);

    // Dashed ghost card border
    QColor dashedColor = isDark ? QColor(0x55, 0x55, 0x55) : QColor(0xD0, 0xD0, 0xD0);
    if ((option.state & QStyle::State_MouseOver) != 0) {
        dashedColor = QColor(0x41, 0x8F, 0xDE);  // Tufts Blue on hover
        dashedColor.setAlphaF(0.6);
    }
    QPen dashedPen(dashedColor);
    dashedPen.setStyle(Qt::DashLine);
    dashedPen.setWidth(2);
    painter->setPen(dashedPen);
    painter->drawRoundedRect(cardRect.adjusted(3, 3, -3, -3), cardRadius, cardRadius);

    // Draw "+" icon in Tufts Blue
    QRect thumbnailRect(cardRect.x() + margin, cardRect.y() + margin, thumbnailSize, thumbnailSize);
    int plusSize = 48;
    QRect plusRect(0, 0, plusSize, plusSize);
    plusRect.moveCenter(thumbnailRect.center());

    QFont plusFont = painter->font();
    plusFont.setPixelSize(36);
    plusFont.setWeight(QFont::Light);
    painter->setFont(plusFont);
    painter->setPen(QColor(0x41, 0x8F, 0xDE));  // Tufts Blue
    painter->drawText(plusRect, Qt::AlignCenter, QStringLiteral("+"));

    // Draw "Open file..." text in secondary color
    QColor textColor = option.palette.text().color();
    textColor.setAlphaF(0.6);
    painter->setPen(textColor);
    QRect textRect(
        cardRect.x() + margin,
        thumbnailRect.bottom() + margin,
        thumbnailSize,
        painter->fontMetrics().lineSpacing()
    );
    painter->setFont(QGuiApplication::font());
    painter->drawText(textRect, Qt::AlignHCenter | Qt::AlignVCenter, tr("Open file..."));

    painter->restore();
}

void FileCardDelegate::paint(
    QPainter* painter,
    const QStyleOptionViewItem& option,
    const QModelIndex& index
) const
{
    auto path = index.data(static_cast<int>(DisplayedFilesModelRoles::path)).toString();

    // Handle sentinel "Open file..." card
    if (path == QLatin1String("__open_file_dialog__")) {
        paintOpenFileCard(painter, option);
        return;
    }

    painter->save();
    // Step 1: Compute card rect (spacing baked into sizeHint, card fills top-left)
    painter->setRenderHint(QPainter::Antialiasing);
    int spacing = static_cast<int>(_parameterGroup->GetInt("FileCardSpacing", 16));
    QRect cardRect = option.rect.adjusted(0, 0, -spacing, -spacing);

    bool isDark = isDarkTheme();

    // Card background and border with theme-aware colors
    QColor cardBg = isDark ? QColor(0x38, 0x38, 0x38) : QColor(Qt::white);
    QColor borderColor = isDark ? QColor(0x55, 0x55, 0x55) : QColor(0xD0, 0xD0, 0xD0);
    if ((option.state & QStyle::State_MouseOver) != 0) {
        cardBg = isDark ? QColor(0x42, 0x42, 0x42) : QColor(0xF5, 0xF5, 0xF5);
        borderColor = QColor(0x41, 0x8F, 0xDE);  // Tufts Blue on hover
        borderColor.setAlphaF(0.6);
    }
    painter->setBrush(cardBg);
    painter->setPen(QPen(borderColor, 2));
    painter->drawRoundedRect(cardRect.adjusted(3, 3, -3, -3), cardRadius, cardRadius);

    // Step 2: Fetch required data
    auto thumbnailSize = static_cast<int>(_parameterGroup->GetInt("FileThumbnailIconsSize", 128));  // NOLINT
    auto baseName = index.data(static_cast<int>(DisplayedFilesModelRoles::baseName)).toString();
    auto size = index.data(static_cast<int>(DisplayedFilesModelRoles::size)).toString();
    auto image = index.data(static_cast<int>(DisplayedFilesModelRoles::image)).toByteArray();

    QPixmap pixmap;
    if (!image.isEmpty()) {
        pixmap.loadFromData(image);
    }
    else {
        pixmap = generateThumbnail(path);
    }

    QPixmap scaledPixmap = pixmap.scaled(
        QSize(thumbnailSize, thumbnailSize),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );

    // Step 3: Positioning
    QRect thumbnailRect(cardRect.x() + margin, cardRect.y() + margin, thumbnailSize, thumbnailSize);

    // Draw thumbnail background area
    QColor thumbBg = isDark ? QColor(0x2E, 0x2E, 0x2E) : QColor(0xF0, 0xF0, 0xF0);
    painter->setBrush(thumbBg);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(thumbnailRect.adjusted(-4, -4, 4, 4), 8, 8);

    // Determine text start X and available width (may shift for pinned indicator)
    int textX = cardRect.x() + margin;
    int textWidth = thumbnailSize;
    constexpr int pinDotSize = 8;
    constexpr int pinDotMargin = 6;

    bool isPinned = false;
    if (_showPinnedIndicator) {
        isPinned = index.data(static_cast<int>(DisplayedFilesModelRoles::pinned)).toBool();
    }

    if (isPinned) {
        textX += pinDotSize + pinDotMargin;
        textWidth -= pinDotSize + pinDotMargin;
    }

    auto elidedName = painter->fontMetrics().elidedText(baseName, Qt::ElideRight, textWidth);

    QRect textRect(
        textX,
        thumbnailRect.bottom() + margin,
        textWidth,
        painter->fontMetrics().lineSpacing()
    );

    // Build size + timestamp string
    QString sizeAndTime = size;
    if (_showTimestamp) {
        auto modifiedTime = index.data(static_cast<int>(DisplayedFilesModelRoles::modifiedTime)).toString();
        auto relTime = relativeTimeString(modifiedTime);
        if (!relTime.isEmpty() && !size.isEmpty()) {
            sizeAndTime = size + QStringLiteral(" \u00B7 ") + relTime;  // " · "
        }
        else if (!relTime.isEmpty()) {
            sizeAndTime = relTime;
        }
    }

    int textAvailableWidth = cardRect.width() - 2 * margin;
    QRect sizeRect(
        cardRect.x() + margin,
        textRect.bottom() + textspacing,
        textAvailableWidth,
        painter->fontMetrics().lineSpacing() + margin
    );

    // Step 4: Draw
    QRect pixmapRect(thumbnailRect.topLeft(), scaledPixmap.size());
    pixmapRect.moveCenter(thumbnailRect.center());
    painter->drawPixmap(pixmapRect.topLeft(), scaledPixmap);

    // Draw pinned indicator (blue dot) before filename
    if (isPinned) {
        QColor accentColor = option.palette.highlight().color();
        painter->setBrush(accentColor);
        painter->setPen(Qt::NoPen);
        int dotY = textRect.center().y() - pinDotSize / 2;
        painter->drawEllipse(cardRect.x() + margin, dotY, pinDotSize, pinDotSize);
        painter->setPen(option.palette.text().color());
    }

    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, elidedName);

    // Draw size/timestamp in slightly lighter color (with proper eliding)
    QColor sizeColor = option.palette.text().color();
    sizeColor.setAlphaF(0.6);
    painter->setPen(sizeColor);
    auto elidedSizeAndTime = painter->fontMetrics().elidedText(
        sizeAndTime, Qt::ElideRight, textAvailableWidth);
    painter->drawText(sizeRect, Qt::AlignLeft | Qt::AlignTop, elidedSizeAndTime);

    painter->restore();
}


QSize FileCardDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto thumbnailSize = _parameterGroup->GetInt("FileThumbnailIconsSize", 128);  // NOLINT
    int spacing = static_cast<int>(_parameterGroup->GetInt("FileCardSpacing", 16));

    QFontMetrics qfm(QGuiApplication::font());
    int textHeight = textspacing + qfm.lineSpacing() * 2;  // name + size/timestamp
    int cardWidth = static_cast<int>(thumbnailSize) + 2 * margin;
    int cardHeight = static_cast<int>(thumbnailSize) + textHeight + 3 * margin;

    return {cardWidth + spacing, cardHeight + spacing};
}

QPixmap FileCardDelegate::generateThumbnail(const QString& path) const
{
    auto thumbnailSize = static_cast<int>(_parameterGroup->GetInt("FileThumbnailIconsSize", 128));  // NOLINT

    // check if we have this thumbnail already inside cache, don't load it once again
    QString cacheKey = getCacheKey(path, thumbnailSize);
    if (!cacheKey.isEmpty()) {
        if (QPixmap* cachedThumbnail = _thumbnailCache.object(cacheKey)) {
            return *cachedThumbnail;  // cache hit - we bail out
        }
    }

    // cache miss - go and load the thumbnail as it could be changed
    return loadAndCacheThumbnail(path, thumbnailSize);
}

QString FileCardDelegate::getCacheKey(const QString& path, int thumbnailSize) const
{
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        return {};
    }

    // create cache key: path:modtime:size
    QString modTime = QString::number(fileInfo.lastModified().toSecsSinceEpoch());
    return QStringLiteral("%1:%2:%3").arg(path, modTime, QString::number(thumbnailSize));
}

QPixmap FileCardDelegate::loadAndCacheThumbnail(const QString& path, int thumbnailSize) const
{
    QPixmap thumbnail;

    if (path.endsWith(QLatin1String(".fcstd"), Qt::CaseSensitivity::CaseInsensitive)) {
        // This is a fallback, the model will have pulled the thumbnail out of the FCStd file if it
        // existed.
        QImageReader reader(QLatin1String(":/icons/freecad-doc.svg"));
        reader.setScaledSize(QSize(thumbnailSize, thumbnailSize));
        thumbnail = QPixmap::fromImage(reader.read());
    }
    else if (path.endsWith(QLatin1String(".fcmacro"), Qt::CaseSensitivity::CaseInsensitive)) {
        QImageReader reader(QLatin1String(":/icons/MacroEditor.svg"));
        reader.setScaledSize(QSize(thumbnailSize, thumbnailSize));
        thumbnail = QPixmap::fromImage(reader.read());
    }
    else if (!QImageReader::imageFormat(path).isEmpty()) {
        // It is an image: it can be its own thumbnail
        QImageReader reader(path);

        // get original size to calculate proper aspect-preserving scaled size
        QSize originalSize = reader.size();
        if (originalSize.isValid()) {
            QSize scaledSize = originalSize.scaled(thumbnailSize, thumbnailSize, Qt::KeepAspectRatio);
            reader.setScaledSize(scaledSize);
        }

        auto image = reader.read();
        if (!image.isNull()) {
            thumbnail = QPixmap::fromImage(image);
        }
        else {
            Base::Console().log(
                "FileCardDelegate: Failed to load image %s: %s\n",
                path.toStdString().c_str(),
                reader.errorString().toStdString().c_str()
            );
        }
    }

    // fallback to system icon if no thumbnail was generated
    if (thumbnail.isNull()) {
        QIcon icon = QFileIconProvider().icon(QFileInfo(path));
        if (!icon.isNull()) {
            thumbnail = icon.pixmap(thumbnailSize);
        }
        else {
            thumbnail = QPixmap(thumbnailSize, thumbnailSize);
            thumbnail.fill();
        }
    }

    // cache the thumbnail if valid
    if (!thumbnail.isNull()) {
        QString cacheKey = getCacheKey(path, thumbnailSize);
        if (!cacheKey.isEmpty()) {
            _thumbnailCache.insert(cacheKey, new QPixmap(thumbnail), 1);
        }
    }

    return thumbnail;
}
