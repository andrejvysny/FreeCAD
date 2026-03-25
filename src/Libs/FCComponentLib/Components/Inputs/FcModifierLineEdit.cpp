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

#include "FcModifierLineEdit.h"

#include <QKeyEvent>
#include <QKeySequence>

#include <FCComponentLib/Components/ComponentMeta.h>
#include <FCComponentLib/Components/ComponentRegistry.h>

namespace FcComponents
{

FcModifierLineEdit::FcModifierLineEdit(QWidget* parent)
    : QLineEdit(parent)
{
    setPlaceholderText(tr("Press modifier keys"));
}

void FcModifierLineEdit::keyPressEvent(QKeyEvent* event)
{
    int key = event->key();
    Qt::KeyboardModifiers state = event->modifiers();

    switch (key) {
        case Qt::Key_Backspace:
        case Qt::Key_Delete:
            clear();
            return;
        case Qt::Key_Control:
        case Qt::Key_Shift:
        case Qt::Key_Alt:
        case Qt::Key_Meta:
            break;
        default:
            return;
    }

    clear();
    QString txtLine;

    if ((state & Qt::ControlModifier) == Qt::ControlModifier) {
        QKeySequence ks(Qt::CTRL);
        txtLine += ks.toString(QKeySequence::NativeText);
    }
    if ((state & Qt::AltModifier) == Qt::AltModifier) {
        QKeySequence ks(Qt::ALT);
        txtLine += ks.toString(QKeySequence::NativeText);
    }
    if ((state & Qt::ShiftModifier) == Qt::ShiftModifier) {
        QKeySequence ks(Qt::SHIFT);
        txtLine += ks.toString(QKeySequence::NativeText);
    }
    if ((state & Qt::MetaModifier) == Qt::MetaModifier) {
        QKeySequence ks(Qt::META);
        txtLine += ks.toString(QKeySequence::NativeText);
    }

    setText(txtLine);
}

}  // namespace FcComponents

FC_REGISTER_COMPONENT(FcModifierLineEdit, "Inputs", "Keyboard modifier input")
