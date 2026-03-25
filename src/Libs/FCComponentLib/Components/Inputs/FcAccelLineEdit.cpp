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

#include "FcAccelLineEdit.h"

#include <QLineEdit>

#include <FCComponentLib/Components/ComponentMeta.h>
#include <FCComponentLib/Components/ComponentRegistry.h>

namespace FcComponents
{

FcAccelLineEdit::FcAccelLineEdit(QWidget* parent)
    : QKeySequenceEdit(parent)
{
    if (auto* le = findChild<QLineEdit*>()) {
        le->setClearButtonEnabled(true);
    }
}

FcAccelLineEdit::FcAccelLineEdit(const QKeySequence& keySequence, QWidget* parent)
    : QKeySequenceEdit(keySequence, parent)
{
    if (auto* le = findChild<QLineEdit*>()) {
        le->setClearButtonEnabled(true);
    }
}

void FcAccelLineEdit::setReadOnly(bool value)
{
    if (auto* le = findChild<QLineEdit*>()) {
        le->setReadOnly(value);
    }
}

bool FcAccelLineEdit::isEmpty() const
{
    return keySequence().isEmpty();
}

QString FcAccelLineEdit::text() const
{
    return keySequence().toString(QKeySequence::NativeText);
}

}  // namespace FcComponents

FC_REGISTER_COMPONENT(FcAccelLineEdit, "Inputs", "Keyboard shortcut input")
