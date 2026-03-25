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

#include <QLabel>

#include <FCComponentLib/FCComponentLibGlobal.h>

namespace FcComponents
{

/** @brief A clickable URL label that opens a link in the system browser.
 *
 * When the user clicks the label, the URL is either opened externally via
 * QDesktopServices or emitted through the linkClicked signal, depending on
 * the launchExternal property.
 */
class FCComponentLibExport FcUrlLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(QString url READ url WRITE setUrl)
    Q_PROPERTY(bool launchExternal READ launchExternal WRITE setLaunchExternal)

public:
    /** @brief Constructs a URL label.
     *  @param parent The parent widget.
     *  @param f Window flags.
     */
    explicit FcUrlLabel(QWidget* parent = nullptr,
                        Qt::WindowFlags f = Qt::WindowFlags());

    /** @brief Destructor. */
    ~FcUrlLabel() override;

    /** @brief Returns the current URL. */
    QString url() const;

    /** @brief Returns whether clicks open the URL externally. */
    bool launchExternal() const;

Q_SIGNALS:
    /** @brief Emitted when the label is clicked and launchExternal is false.
     *  @param url The URL string.
     */
    void linkClicked(QString url);

public Q_SLOTS:
    /** @brief Sets the URL to open when clicked.
     *  @param u The URL string.
     */
    void setUrl(const QString& u);

    /** @brief Sets whether clicks open the URL in an external browser.
     *  @param l True to launch externally, false to emit linkClicked.
     */
    void setLaunchExternal(bool l);

protected:
    /** @brief Opens the URL or emits linkClicked on mouse release. */
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QString m_url;
    bool m_launchExternal;
};

}  // namespace FcComponents
