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

#include "FcButtonGroup.h"

#include <QAbstractButton>

namespace FcComponents
{

FcButtonGroup::FcButtonGroup(QObject* parent)
    : QButtonGroup(parent)
    , m_exclusive(true)
{
    QButtonGroup::setExclusive(false);

    connect(
        this,
        qOverload<QAbstractButton*>(&QButtonGroup::buttonClicked),
        [this](QAbstractButton* button) {
            if (exclusive()) {
                const auto btns = buttons();
                for (auto* btn : btns) {
                    if (btn && btn != button && btn->isCheckable()) {
                        btn->setChecked(false);
                    }
                }
            }
        }
    );
}

void FcButtonGroup::setExclusive(bool on)
{
    m_exclusive = on;
}

bool FcButtonGroup::exclusive() const
{
    return m_exclusive;
}

}  // namespace FcComponents
