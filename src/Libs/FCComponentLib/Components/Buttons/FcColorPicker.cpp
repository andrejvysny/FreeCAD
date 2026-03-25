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

#include <cmath>

#include <QApplication>
#include <QColorDialog>
#include <QGridLayout>
#include <QLayout>
#include <QPainter>
#include <QPushButton>
#include <QStyle>
#include <QtCore/QMap>
#include <QtGui/QFocusEvent>
#include <QtGui/QHideEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPaintEvent>
#include <QtGui/QPixmap>
#include <QtGui/QShowEvent>

#include "FcColorPicker.h"
#include "FcColorPickerPrivate.h"

#include <FCComponentLib/Components/ComponentMeta.h>
#include <FCComponentLib/Components/ComponentRegistry.h>

namespace FcComponents
{

// ---------------------------------------------------------------------------
// FcColorPicker
// ---------------------------------------------------------------------------

FcColorPicker::FcColorPicker(QWidget* parent, int columns, bool enableColorDialog)
    : QPushButton(parent)
    , m_popup(nullptr)
    , m_withColorDialog(enableColorDialog)
{
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    setAutoDefault(false);
    setAutoFillBackground(true);
    setCheckable(true);

    setText(tr("Black"));
    m_firstInserted = false;

    m_color = Qt::black;
    m_dirty = true;

    m_popup = new ColorPickerPopup(columns, m_withColorDialog, this);
    connect(m_popup, &ColorPickerPopup::selected, this, &FcColorPicker::setCurrentColor);
    connect(m_popup, &ColorPickerPopup::hid, this, &FcColorPicker::popupClosed);

    connect(this, &FcColorPicker::toggled, this, &FcColorPicker::buttonPressed);
}

FcColorPicker::~FcColorPicker() = default;

void FcColorPicker::buttonPressed(bool toggled)
{
    if (!toggled) {
        return;
    }

    const QRect desktop = QApplication::activeWindow()->geometry();

    // Make sure the popup is inside the desktop.
    QPoint pos = mapToGlobal(rect().bottomLeft());
    if (pos.x() < desktop.left()) {
        pos.setX(desktop.left());
    }
    if (pos.y() < desktop.top()) {
        pos.setY(desktop.top());
    }

    if ((pos.x() + m_popup->sizeHint().width()) > desktop.right()) {
        pos.setX(desktop.right() - m_popup->sizeHint().width());
    }
    if ((pos.y() + m_popup->sizeHint().height()) > desktop.bottom()) {
        pos.setY(desktop.bottom() - m_popup->sizeHint().height());
    }
    m_popup->move(pos);

    if (auto* item = m_popup->find(m_color)) {
        item->setSelected(true);
    }

    // Remove focus from this widget, preventing the focus rect
    // from showing when the popup is shown.
    clearFocus();
    update();

    // Allow keyboard navigation as soon as the popup shows.
    m_popup->setFocus();

    m_popup->show();
}

void FcColorPicker::paintEvent(QPaintEvent* e)
{
    if (m_dirty) {
        int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize);
        QPixmap pix(iconSize, iconSize);
        pix.fill(palette().button().color());

        QPainter p(&pix);
        int w = pix.width();
        int h = pix.height();
        p.setPen(QPen(Qt::gray));
        p.setBrush(m_color);
        p.drawRect(2, 2, w - 5, h - 5);
        setIcon(QIcon(pix));

        m_dirty = false;
    }
    QPushButton::paintEvent(e);
}

void FcColorPicker::popupClosed()
{
    setChecked(false);
    setFocus();
}

QColor FcColorPicker::currentColor() const
{
    return m_color;
}

QColor FcColorPicker::color(int index) const
{
    return m_popup->color(index);
}

void FcColorPicker::setStandardColors()
{
    insertColor(Qt::black, tr("Black"));
    insertColor(Qt::white, tr("White"));
    insertColor(Qt::red, tr("Red"));
    insertColor(Qt::darkRed, tr("Dark red"));
    insertColor(Qt::green, tr("Green"));
    insertColor(Qt::darkGreen, tr("Dark green"));
    insertColor(Qt::blue, tr("Blue"));
    insertColor(Qt::darkBlue, tr("Dark blue"));
    insertColor(Qt::cyan, tr("Cyan"));
    insertColor(Qt::darkCyan, tr("Dark cyan"));
    insertColor(Qt::magenta, tr("Magenta"));
    insertColor(Qt::darkMagenta, tr("Dark magenta"));
    insertColor(Qt::yellow, tr("Yellow"));
    insertColor(Qt::darkYellow, tr("Dark yellow"));
    insertColor(Qt::gray, tr("Gray"));
    insertColor(Qt::darkGray, tr("Dark gray"));
    insertColor(Qt::lightGray, tr("Light gray"));
}

void FcColorPicker::setCurrentColor(const QColor& color)
{
    if (color.isValid() && m_color == color) {
        Q_EMIT colorSet(color);
        return;
    }
    if (m_color == color || !color.isValid()) {
        return;
    }

    auto* item = m_popup->find(color);
    if (!item) {
        insertColor(color, tr("Custom Color"));
        item = m_popup->find(color);
    }

    m_popup->setLastSel(color);

    m_color = color;
    setText(item->text());

    m_dirty = true;

    m_popup->hide();
    repaint();

    item->setSelected(true);
    Q_EMIT colorChanged(color);
    Q_EMIT colorSet(color);
}

void FcColorPicker::insertColor(const QColor& color, const QString& text, int index)
{
    m_popup->insertColor(color, text, index);
    if (!m_firstInserted) {
        m_color = color;
        setText(text);
        m_firstInserted = true;
    }
}

void FcColorPicker::setColorDialogEnabled(bool enabled)
{
    m_withColorDialog = enabled;
}

bool FcColorPicker::colorDialogEnabled() const
{
    return m_withColorDialog;
}

QColor FcColorPicker::getColor(const QPoint& point, bool allowCustomColors)
{
    ColorPickerPopup popup(-1, allowCustomColors);

    popup.insertColor(Qt::black, tr("Black"), 0);
    popup.insertColor(Qt::white, tr("White"), 1);
    popup.insertColor(Qt::red, tr("Red"), 2);
    popup.insertColor(Qt::darkRed, tr("Dark red"), 3);
    popup.insertColor(Qt::green, tr("Green"), 4);
    popup.insertColor(Qt::darkGreen, tr("Dark green"), 5);
    popup.insertColor(Qt::blue, tr("Blue"), 6);
    popup.insertColor(Qt::darkBlue, tr("Dark blue"), 7);
    popup.insertColor(Qt::cyan, tr("Cyan"), 8);
    popup.insertColor(Qt::darkCyan, tr("Dark cyan"), 9);
    popup.insertColor(Qt::magenta, tr("Magenta"), 10);
    popup.insertColor(Qt::darkMagenta, tr("Dark magenta"), 11);
    popup.insertColor(Qt::yellow, tr("Yellow"), 12);
    popup.insertColor(Qt::darkYellow, tr("Dark yellow"), 13);
    popup.insertColor(Qt::gray, tr("Gray"), 14);
    popup.insertColor(Qt::darkGray, tr("Dark gray"), 15);
    popup.insertColor(Qt::lightGray, tr("Light gray"), 16);

    popup.move(point);
    popup.exec();
    return popup.lastSelected();
}

}  // namespace FcComponents

FC_REGISTER_COMPONENT(FcColorPicker, "Buttons", "Color picker button with popup grid")
