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

#include <QWidget>

#include <FCComponentLib/FCComponentLibGlobal.h>

class QGridLayout;
class QVBoxLayout;
class QLabel;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;
class QSpacerItem;

namespace FcComponents
{

/** @brief Dual-panel item selector with add/remove/up/down controls.
 *
 * Provides two QTreeWidget panels (available and selected) with buttons
 * to move items between panels and reorder items in the selected panel.
 * Supports keyboard shortcuts (Ctrl+Arrow keys) and double-click to
 * transfer items between panels.
 */
class FCComponentLibExport FcActionSelector : public QWidget
{
    Q_OBJECT

public:
    /** @brief Constructs an action selector widget.
     *  @param parent The parent widget.
     */
    explicit FcActionSelector(QWidget* parent = nullptr);

    /** @brief Destructor. */
    ~FcActionSelector() override;

    /** @brief Returns the tree widget for available items.
     *  @return Pointer to the available items tree widget.
     */
    QTreeWidget* availableTreeWidget() const;

    /** @brief Returns the tree widget for selected items.
     *  @return Pointer to the selected items tree widget.
     */
    QTreeWidget* selectedTreeWidget() const;

    /** @brief Sets the label text for the selected items panel.
     *  @param label The label text to display.
     */
    void setSelectedLabel(const QString& label);

    /** @brief Returns the label text for the selected items panel.
     *  @return The current selected panel label text.
     */
    QString selectedLabel() const;

    /** @brief Sets the label text for the available items panel.
     *  @param label The label text to display.
     */
    void setAvailableLabel(const QString& label);

    /** @brief Returns the label text for the available items panel.
     *  @return The current available panel label text.
     */
    QString availableLabel() const;

private:
    void keyPressEvent(QKeyEvent* event) override;
    void changeEvent(QEvent* event) override;
    void retranslateUi();
    void setButtonsEnabled();

private Q_SLOTS:
    /** @brief Moves the current item from available to selected panel. */
    void onAddButtonClicked();

    /** @brief Moves the current item from selected to available panel. */
    void onRemoveButtonClicked();

    /** @brief Moves the current selected item up one position. */
    void onUpButtonClicked();

    /** @brief Moves the current selected item down one position. */
    void onDownButtonClicked();

    /** @brief Updates button enabled states when selection changes. */
    void onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

    /** @brief Handles double-click to transfer items between panels. */
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);

private:
    QGridLayout* m_gridLayout;
    QVBoxLayout* m_vboxLayout;
    QVBoxLayout* m_vboxLayout1;
    QPushButton* m_addButton;
    QPushButton* m_removeButton;
    QPushButton* m_upButton;
    QPushButton* m_downButton;
    QLabel* m_labelAvailable;
    QLabel* m_labelSelected;
    QTreeWidget* m_availableWidget;
    QTreeWidget* m_selectedWidget;
    QSpacerItem* m_spacerItem;
    QSpacerItem* m_spacerItem1;
};

}  // namespace FcComponents
