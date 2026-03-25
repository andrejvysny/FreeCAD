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

#include <QVariant>
#include <QWidget>

#include <FCComponentLib/FCComponentLibGlobal.h>

class QLabel;
class QPushButton;

namespace FcComponents
{

/** @brief A label with an action button on the right side.
 *
 * Provides a composite widget consisting of a QLabel and a QPushButton.
 * The button displays an ellipsis character by default and can trigger
 * a browse action. The value property holds a QVariant that is displayed
 * in the label.
 */
class FCComponentLibExport FcLabelButton : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)

public:
    /** @brief Constructs a label button widget.
     *  @param parent The parent widget.
     */
    explicit FcLabelButton(QWidget* parent = nullptr);

    /** @brief Destructor. */
    ~FcLabelButton() override;

    /** @brief Returns the current value.
     *  @return The current QVariant value.
     */
    QVariant value() const;

    /** @brief Returns the label widget.
     *  @return Pointer to the internal QLabel.
     */
    QLabel* getLabel() const;

    /** @brief Returns the button widget.
     *  @return Pointer to the internal QPushButton.
     */
    QPushButton* getButton() const;

public Q_SLOTS:
    /** @brief Sets the current value.
     *  @param val The new value to set.
     */
    void setValue(const QVariant& val);

protected:
    /** @brief Displays the given value in the label.
     *
     *  Override this method to customize how the value is displayed.
     *  @param data The value to display.
     */
    virtual void showValue(const QVariant& data);

    /** @brief Handles resize events to keep button square.
     *  @param event The resize event.
     */
    void resizeEvent(QResizeEvent* event) override;

protected Q_SLOTS:
    /** @brief Called when the button is clicked.
     *
     *  Override this method to implement custom browse behavior.
     */
    virtual void browse();

Q_SIGNALS:
    /** @brief Emitted when the value changes.
     *  @param val The new value.
     */
    void valueChanged(const QVariant& val);

    /** @brief Emitted when the button is clicked. */
    void buttonClicked();

private:
    QLabel* m_label;
    QPushButton* m_button;
    QVariant m_value;
};

}  // namespace FcComponents
