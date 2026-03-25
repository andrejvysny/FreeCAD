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

class QLineEdit;
class QPushButton;

namespace FcComponents
{

/** @brief Editable label with configurable input types and a browse button.
 *
 * Provides a composite widget consisting of a QLineEdit and a QPushButton.
 * The input can be configured to accept strings, floating-point numbers,
 * or integers. The button opens a multi-line editor dialog for list input.
 * Text is displayed in bracket notation (e.g., "[item1, item2]").
 */
class FCComponentLibExport FcLabelEditor : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString buttonText READ buttonText WRITE setButtonText)

public:
    /** @brief Input validation type for the editor. */
    enum InputType
    {
        String,  ///< Accept any string input.
        Float,   ///< Accept floating-point number input.
        Integer  ///< Accept integer number input.
    };

    /** @brief Constructs a label editor widget.
     *  @param parent The parent widget.
     */
    explicit FcLabelEditor(QWidget* parent = nullptr);

    /** @brief Destructor. */
    ~FcLabelEditor() override;

    /** @brief Returns the current plain text content.
     *  @return The text without bracket notation.
     */
    QString text() const;

    /** @brief Returns the browse button's text.
     *  @return The current button text.
     */
    QString buttonText() const;

    /** @brief Sets the input validation type.
     *  @param type The input type to use for validation.
     */
    void setInputType(InputType type);

public Q_SLOTS:
    /** @brief Sets the text content.
     *  @param text The text to set (displayed in bracket notation).
     */
    virtual void setText(const QString& text);

    /** @brief Sets the browse button's text.
     *  @param text The button text to display.
     */
    virtual void setButtonText(const QString& text);

    /** @brief Validates the text input and updates the plain text.
     *  @param text The text from the line edit to validate.
     */
    virtual void validateText(const QString& text);

Q_SIGNALS:
    /** @brief Emitted when the text content changes.
     *  @param text The new plain text content.
     */
    void textChanged(const QString& text);

private Q_SLOTS:
    /** @brief Opens a multi-line editor dialog for list editing. */
    void changeText();

protected:
    /** @brief Handles resize events to keep button square.
     *  @param event The resize event.
     */
    void resizeEvent(QResizeEvent* event) override;

private:
    InputType m_type;
    QString m_plainText;
    QLineEdit* m_lineEdit;
    QPushButton* m_button;
};

}  // namespace FcComponents
