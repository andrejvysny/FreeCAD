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

#include "FcClearLineEdit.h"

#include <QAction>
#include <QIcon>
#include <QResizeEvent>

#include <FCComponentLib/Components/ComponentMeta.h>
#include <FCComponentLib/Components/ComponentRegistry.h>

namespace FcComponents
{

FcClearLineEdit::FcClearLineEdit(QWidget* parent)
    : QLineEdit(parent)
{
    m_clearAction = addAction(
        QIcon(QStringLiteral(":/icons/edit-cleartext.svg")),
        QLineEdit::TrailingPosition
    );
    connect(m_clearAction, &QAction::triggered, this, &FcClearLineEdit::clear);
    connect(this, &QLineEdit::textChanged, this, &FcClearLineEdit::updateClearButton);
}

void FcClearLineEdit::resizeEvent(QResizeEvent* event)
{
    QLineEdit::resizeEvent(event);
}

void FcClearLineEdit::updateClearButton(const QString& text)
{
    m_clearAction->setVisible(!text.isEmpty());
}

}  // namespace FcComponents

FC_REGISTER_COMPONENT(FcClearLineEdit, "Inputs", "Line edit with clear button")
