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

#include "FcUrlLabel.h"

#include <QApplication>
#include <QDesktopServices>
#include <QMouseEvent>
#include <QUrl>

#include <FCComponentLib/Components/ComponentMeta.h>
#include <FCComponentLib/Components/ComponentRegistry.h>

namespace FcComponents
{

FcUrlLabel::FcUrlLabel(QWidget* parent, Qt::WindowFlags f)
    : QLabel(parent, f)
    , m_url(QStringLiteral("http://localhost"))
    , m_launchExternal(true)
{
    setToolTip(m_url);
    setCursor(Qt::PointingHandCursor);
    if (qApp->styleSheet().isEmpty()) {
        setStyleSheet(
            QStringLiteral("FcComponents--FcUrlLabel "
                           "{color: #0000FF;text-decoration: underline;}")
        );
    }
}

FcUrlLabel::~FcUrlLabel() = default;

QString FcUrlLabel::url() const
{
    return m_url;
}

bool FcUrlLabel::launchExternal() const
{
    return m_launchExternal;
}

void FcUrlLabel::setUrl(const QString& u)
{
    m_url = u;
    setToolTip(m_url);
}

void FcUrlLabel::setLaunchExternal(bool l)
{
    m_launchExternal = l;
}

void FcUrlLabel::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    if (m_launchExternal) {
        QDesktopServices::openUrl(QUrl(m_url));
    }
    else {
        Q_EMIT linkClicked(m_url);
    }
}

}  // namespace FcComponents

FC_REGISTER_COMPONENT(FcUrlLabel, "Display", "Clickable URL label")
