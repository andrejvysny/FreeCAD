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

#include <QMenu>
#include <QPushButton>
#include <QToolButton>
#include <QWidget>

#include <FCComponentLib/FCComponentLibGlobal.h>

namespace FcComponents
{

/// Split button widget combining a primary push button with a dropdown menu button.
///
/// The widget presents a QPushButton for the default action alongside a QToolButton
/// that opens a QMenu when clicked, visually styled as a single composite control.
class FCComponentLibExport FcSplitButton : public QWidget
{
    Q_OBJECT

public:
    /** @brief Construct a split button with no text. */
    explicit FcSplitButton(QWidget* parent = nullptr);

    /** @brief Construct a split button with the given text on the main button. */
    explicit FcSplitButton(const QString& text, QWidget* parent = nullptr);

    /** @brief Return the primary push button. */
    QPushButton* mainButton() const;

    /** @brief Return the dropdown tool button. */
    QToolButton* menuButton() const;

    /** @brief Return the dropdown menu. */
    QMenu* menu() const;

Q_SIGNALS:
    /** @brief Emitted when the main button is clicked. */
    void defaultClicked();

    /** @brief Emitted when a menu action is triggered. */
    void triggered(QAction* action);

private:
    QPushButton* m_main;
    QToolButton* m_menuButton;
    QMenu* m_menu;
};

}  // namespace FcComponents
