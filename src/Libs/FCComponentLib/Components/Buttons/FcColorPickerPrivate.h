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
#include <QEventLoop>
#include <QFrame>
#include <QGridLayout>
#include <QList>
#include <QMap>
#include <QString>

namespace FcComponents
{

/** @brief A single color swatch item in the color picker popup grid. */
class ColorPickerItem : public QFrame
{
    Q_OBJECT

public:
    /** @brief Construct a color item with the given color and display text. */
    ColorPickerItem(const QColor& color = Qt::white,
                    const QString& text = QString(),
                    QWidget* parent = nullptr);

    ~ColorPickerItem() override;

    /** @brief Return the item's color. */
    QColor color() const;

    /** @brief Return the item's display text. */
    QString text() const;

    /** @brief Set whether this item is visually selected. */
    void setSelected(bool selected);

    /** @brief Return whether this item is visually selected. */
    bool isSelected() const;

Q_SIGNALS:
    /** @brief Emitted on mouse click. */
    void clicked();

    /** @brief Emitted when the item is selected by the user. */
    void selected();

public Q_SLOTS:
    /** @brief Change the item's color and text. */
    void setColor(const QColor& color, const QString& text = QString());

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    QColor m_color;
    QString m_text;
    bool m_selected;
};

/** @brief An unstyled button that renders an ellipsis ("...") for the "More" action. */
class ColorPickerButton : public QFrame
{
    Q_OBJECT

public:
    /** @brief Construct a "more" button. */
    explicit ColorPickerButton(QWidget* parent);

Q_SIGNALS:
    /** @brief Emitted when the button is clicked or activated via keyboard. */
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    void focusInEvent(QFocusEvent* e) override;
    void focusOutEvent(QFocusEvent* e) override;
};

/** @brief Popup frame containing a grid of ColorPickerItem widgets. */
class ColorPickerPopup : public QFrame
{
    Q_OBJECT

public:
    /** @brief Construct a color grid popup.
     *
     *  @param width           Number of columns (-1 = auto-calculate).
     *  @param withColorDialog Whether the "More" button is shown.
     *  @param parent          Parent widget.
     */
    ColorPickerPopup(int width, bool withColorDialog, QWidget* parent = nullptr);

    ~ColorPickerPopup() override;

    /** @brief Insert a color into the grid.
     *
     *  @param col   The color to add.
     *  @param text  Display name.
     *  @param index Position (-1 appends).
     */
    void insertColor(const QColor& col, const QString& text, int index);

    /** @brief Show the popup and run a local event loop until it closes. */
    void exec();

    /** @brief Return the last color that was selected. */
    QColor lastSelected() const;

    /** @brief Find the item matching the given color, or nullptr. */
    ColorPickerItem* find(const QColor& col) const;

    /** @brief Return the color at the given grid index. */
    QColor color(int index) const;

    /** @brief Set the last-selected color without triggering signals. */
    void setLastSel(const QColor& col);

Q_SIGNALS:
    /** @brief Emitted when a color is picked. */
    void selected(const QColor&);

    /** @brief Emitted when the popup hides. */
    void hid();

public Q_SLOTS:
    /** @brief Open a QColorDialog and insert the chosen color. */
    void getColorFromDialog();

protected Q_SLOTS:
    void updateSelected();

protected:
    void keyPressEvent(QKeyEvent* e) override;
    void showEvent(QShowEvent* e) override;
    void hideEvent(QHideEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

    /** @brief Rebuild the grid layout from the current item list. */
    void regenerateGrid();

private:
    /** @brief Handle Enter/Space key selection in the grid. */
    void handleEnterKey(int curRow, int curCol);

    QMap<int, QMap<int, QWidget*>> m_widgetAt;
    QList<ColorPickerItem*> m_items;
    QGridLayout* m_grid;
    ColorPickerButton* m_moreButton;
    QEventLoop* m_eventLoop;

    int m_lastPos;
    int m_cols;
    QColor m_lastSel;
};

}  // namespace FcComponents
