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

/// @file FcColorPickerPrivate.cpp
/// Implementation of internal helper widgets for FcColorPicker:
/// ColorPickerPopup, ColorPickerItem, and ColorPickerButton.

#include <cmath>

#include <QApplication>
#include <QColorDialog>
#include <QGridLayout>
#include <QPainter>
#include <QPushButton>
#include <QtGui/QFocusEvent>
#include <QtGui/QHideEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPaintEvent>
#include <QtGui/QShowEvent>

#include "FcColorPickerPrivate.h"

namespace FcComponents
{

// ---------------------------------------------------------------------------
// ColorPickerPopup
// ---------------------------------------------------------------------------

ColorPickerPopup::ColorPickerPopup(int width, bool withColorDialog, QWidget* parent)
    : QFrame(parent, Qt::Popup)
{
    setFrameStyle(QFrame::StyledPanel);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setProperty("class", "popup");

    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    m_cols = width;

    if (withColorDialog) {
        m_moreButton = new ColorPickerButton(this);
        m_moreButton->setFixedWidth(24);
        m_moreButton->setFixedHeight(21);
        m_moreButton->setFrameRect(QRect(2, 2, 20, 17));
        connect(m_moreButton,
                &ColorPickerButton::clicked,
                this,
                &ColorPickerPopup::getColorFromDialog);
    }
    else {
        m_moreButton = nullptr;
    }

    m_eventLoop = nullptr;
    m_grid = nullptr;
    regenerateGrid();
}

ColorPickerPopup::~ColorPickerPopup()
{
    if (m_eventLoop) {
        m_eventLoop->exit();
    }
}

ColorPickerItem* ColorPickerPopup::find(const QColor& col) const
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i) && m_items.at(i)->color() == col) {
            return m_items.at(i);
        }
    }
    return nullptr;
}

void ColorPickerPopup::insertColor(const QColor& col, const QString& text, int index)
{
    // Don't add colors that we already have.
    auto* existingItem = find(col);
    auto* lastSelectedItem = find(lastSelected());

    if (existingItem) {
        if (lastSelectedItem && existingItem != lastSelectedItem) {
            lastSelectedItem->setSelected(false);
        }
        existingItem->setFocus();
        existingItem->setSelected(true);
        return;
    }

    auto* item = new ColorPickerItem(col, text, this);

    if (lastSelectedItem) {
        lastSelectedItem->setSelected(false);
    }
    else {
        item->setSelected(true);
        m_lastSel = col;
    }
    item->setFocus();

    connect(item, &ColorPickerItem::selected, this, &ColorPickerPopup::updateSelected);

    if (index == -1) {
        index = m_items.count();
    }

    m_items.insert(static_cast<int>(index), item);
    regenerateGrid();

    update();
}

QColor ColorPickerPopup::color(int index) const
{
    if (index < 0 || index > m_items.count() - 1) {
        return {};
    }
    return m_items.at(index)->color();
}

void ColorPickerPopup::exec()
{
    show();

    QEventLoop e;
    m_eventLoop = &e;
    (void)e.exec();
    m_eventLoop = nullptr;
}

void ColorPickerPopup::updateSelected()
{
    QLayoutItem* layoutItem;
    int i = 0;
    while ((layoutItem = m_grid->itemAt(i)) != nullptr) {
        if (auto* litem = qobject_cast<ColorPickerItem*>(layoutItem->widget())) {
            if (litem != sender()) {
                litem->setSelected(false);
            }
        }
        ++i;
    }

    if (auto* item = qobject_cast<ColorPickerItem*>(sender())) {
        m_lastSel = item->color();
        Q_EMIT selected(item->color());
    }

    hide();
}

void ColorPickerPopup::mouseReleaseEvent(QMouseEvent* e)
{
    if (!rect().contains(e->pos())) {
        hide();
    }
}

void ColorPickerPopup::keyPressEvent(QKeyEvent* e)
{
    int curRow = 0;
    int curCol = 0;

    bool foundFocus = false;
    for (int j = 0; !foundFocus && j < m_grid->rowCount(); ++j) {
        for (int i = 0; !foundFocus && i < m_grid->columnCount(); ++i) {
            if (m_widgetAt[j][i] && m_widgetAt[j][i]->hasFocus()) {
                curRow = j;
                curCol = i;
                foundFocus = true;
                break;
            }
        }
    }

    switch (e->key()) {
    case Qt::Key_Left:
        if (curCol > 0) {
            --curCol;
        }
        else if (curRow > 0) {
            --curRow;
            curCol = m_grid->columnCount() - 1;
        }
        break;
    case Qt::Key_Right:
        if (curCol < m_grid->columnCount() - 1 && m_widgetAt[curRow][curCol + 1]) {
            ++curCol;
        }
        else if (curRow < m_grid->rowCount() - 1) {
            ++curRow;
            curCol = 0;
        }
        break;
    case Qt::Key_Up:
        if (curRow > 0) {
            --curRow;
        }
        else {
            curCol = 0;
        }
        break;
    case Qt::Key_Down:
        if (curRow < m_grid->rowCount() - 1) {
            QWidget* w = m_widgetAt[curRow + 1][curCol];
            if (w) {
                ++curRow;
            }
            else {
                for (int i = 1; i < m_grid->columnCount(); ++i) {
                    if (!m_widgetAt[curRow + 1][i]) {
                        curCol = i - 1;
                        ++curRow;
                        break;
                    }
                }
            }
        }
        break;
    case Qt::Key_Space:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        handleEnterKey(curRow, curCol);
        break;
    case Qt::Key_Escape:
        hide();
        break;
    default:
        e->ignore();
        break;
    }

    m_widgetAt[curRow][curCol]->setFocus();
}

void ColorPickerPopup::handleEnterKey(int curRow, int curCol)
{
    QWidget* w = m_widgetAt[curRow][curCol];
    if (!w) {
        return;
    }

    auto* wi = qobject_cast<ColorPickerItem*>(w);
    if (!wi && qobject_cast<QPushButton*>(w)) {
        // Treat QPushButton the same way (legacy compatibility).
        wi = static_cast<ColorPickerItem*>(w);
    }
    if (wi) {
        wi->setSelected(true);

        QLayoutItem* layoutItem;
        int i = 0;
        while ((layoutItem = m_grid->itemAt(i)) != nullptr) {
            if (auto* litem = qobject_cast<ColorPickerItem*>(layoutItem->widget())) {
                if (litem != wi) {
                    litem->setSelected(false);
                }
            }
            ++i;
        }

        m_lastSel = wi->color();
        Q_EMIT selected(wi->color());
        hide();
    }
}

void ColorPickerPopup::hideEvent(QHideEvent* e)
{
    if (m_eventLoop) {
        m_eventLoop->exit();
    }

    setFocus();

    Q_EMIT hid();
    QFrame::hideEvent(e);
}

QColor ColorPickerPopup::lastSelected() const
{
    return m_lastSel;
}

void ColorPickerPopup::showEvent(QShowEvent*)
{
    bool foundSelected = false;
    for (int i = 0; i < m_grid->columnCount(); ++i) {
        for (int j = 0; j < m_grid->rowCount(); ++j) {
            QWidget* w = m_widgetAt[j][i];
            if (auto* cpi = qobject_cast<ColorPickerItem*>(w)) {
                if (cpi->isSelected()) {
                    w->setFocus();
                    foundSelected = true;
                    break;
                }
            }
        }
    }

    if (!foundSelected) {
        if (m_items.isEmpty()) {
            setFocus();
        }
        else {
            m_widgetAt[0][0]->setFocus();
        }
    }
}

void ColorPickerPopup::regenerateGrid()
{
    m_widgetAt.clear();

    int columns = m_cols;
    if (columns == -1) {
        columns = static_cast<int>(ceil(sqrt(static_cast<float>(m_items.count()))));
    }

    // When the number of columns grows, the number of rows will
    // fall. There's no way to shrink a grid, so we create a new one.
    delete m_grid;
    m_grid = new QGridLayout(this);
    m_grid->setContentsMargins(1, 1, 1, 1);
    m_grid->setSpacing(0);

    int ccol = 0;
    int crow = 0;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i)) {
            m_widgetAt[crow][ccol] = m_items.at(i);
            m_grid->addWidget(m_items.at(i), crow, ccol++);
            if (ccol == columns) {
                ++crow;
                ccol = 0;
            }
        }
    }

    if (m_moreButton) {
        m_grid->addWidget(m_moreButton, crow, ccol);
        m_widgetAt[crow][ccol] = m_moreButton;
    }
    updateGeometry();
}

void ColorPickerPopup::getColorFromDialog()
{
    QColor col =
        QColorDialog::getColor(m_lastSel, parentWidget(), QString(),
                               QColorDialog::ShowAlphaChannel);
    if (!col.isValid()) {
        return;
    }

    insertColor(col, tr("Custom Color"), -1);
    m_lastSel = col;
    Q_EMIT selected(col);
}

void ColorPickerPopup::setLastSel(const QColor& col)
{
    m_lastSel = col;
}

// ---------------------------------------------------------------------------
// ColorPickerItem
// ---------------------------------------------------------------------------

ColorPickerItem::ColorPickerItem(const QColor& color, const QString& text, QWidget* parent)
    : QFrame(parent)
    , m_color(color)
    , m_text(text)
    , m_selected(false)
{
    setToolTip(m_text);
    setFixedWidth(24);
    setFixedHeight(21);
}

ColorPickerItem::~ColorPickerItem() = default;

QColor ColorPickerItem::color() const
{
    return m_color;
}

QString ColorPickerItem::text() const
{
    return m_text;
}

bool ColorPickerItem::isSelected() const
{
    return m_selected;
}

void ColorPickerItem::setSelected(bool selected)
{
    m_selected = selected;
    update();
}

void ColorPickerItem::setColor(const QColor& color, const QString& text)
{
    m_color = color;
    m_text = text;
    setToolTip(m_text);
    update();
}

void ColorPickerItem::mouseMoveEvent(QMouseEvent*)
{
    setFocus();
    update();
}

void ColorPickerItem::mouseReleaseEvent(QMouseEvent*)
{
    m_selected = true;
    Q_EMIT selected();
}

void ColorPickerItem::mousePressEvent(QMouseEvent*)
{
    setFocus();
    update();
}

void ColorPickerItem::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    int w = width();
    int h = height();

    p.setPen(QPen(Qt::gray, 0, Qt::SolidLine));

    if (m_selected) {
        p.drawRect(1, 1, w - 3, h - 3);
    }

    p.setPen(QPen(Qt::black, 0, Qt::SolidLine));
    p.drawRect(3, 3, w - 7, h - 7);
    p.fillRect(QRect(4, 4, w - 8, h - 8), QBrush(m_color));

    if (hasFocus()) {
        p.drawRect(0, 0, w - 1, h - 1);
    }
}

// ---------------------------------------------------------------------------
// ColorPickerButton
// ---------------------------------------------------------------------------

ColorPickerButton::ColorPickerButton(QWidget* parent)
    : QFrame(parent)
{
    setFrameStyle(StyledPanel);
}

void ColorPickerButton::mousePressEvent(QMouseEvent*)
{
    setFrameShadow(Sunken);
    update();
}

void ColorPickerButton::mouseMoveEvent(QMouseEvent*)
{
    setFocus();
    update();
}

void ColorPickerButton::mouseReleaseEvent(QMouseEvent*)
{
    setFrameShadow(Raised);
    repaint();
    Q_EMIT clicked();
}

void ColorPickerButton::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down || e->key() == Qt::Key_Left
        || e->key() == Qt::Key_Right) {
        qApp->sendEvent(parent(), e);
    }
    else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Space
             || e->key() == Qt::Key_Return) {
        setFrameShadow(Sunken);
        update();
    }
    else {
        QFrame::keyPressEvent(e);
    }
}

void ColorPickerButton::keyReleaseEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down || e->key() == Qt::Key_Left
        || e->key() == Qt::Key_Right) {
        qApp->sendEvent(parent(), e);
    }
    else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Space
             || e->key() == Qt::Key_Return) {
        setFrameShadow(Raised);
        repaint();
        Q_EMIT clicked();
    }
    else {
        QFrame::keyReleaseEvent(e);
    }
}

void ColorPickerButton::focusInEvent(QFocusEvent* e)
{
    setFrameShadow(Raised);
    update();
    QFrame::focusInEvent(e);
}

void ColorPickerButton::focusOutEvent(QFocusEvent* e)
{
    setFrameShadow(Raised);
    update();
    QFrame::focusOutEvent(e);
}

void ColorPickerButton::paintEvent(QPaintEvent* e)
{
    QFrame::paintEvent(e);

    QPainter p(this);
    p.fillRect(contentsRect(), palette().button());

    QRect r = rect();

    int offset = frameShadow() == Sunken ? 1 : 0;

    QPen pen(palette().buttonText(), 1);
    p.setPen(pen);

    p.drawRect(r.center().x() + offset - 4, r.center().y() + offset, 1, 1);
    p.drawRect(r.center().x() + offset, r.center().y() + offset, 1, 1);
    p.drawRect(r.center().x() + offset + 4, r.center().y() + offset, 1, 1);
    if (hasFocus()) {
        p.setPen(QPen(Qt::black, 0, Qt::SolidLine));
        p.drawRect(0, 0, width() - 1, height() - 1);
    }

    p.end();
}

}  // namespace FcComponents
