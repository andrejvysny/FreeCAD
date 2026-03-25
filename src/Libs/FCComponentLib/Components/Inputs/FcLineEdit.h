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

#include <QLineEdit>

#include <FCComponentLib/FCComponentLibGlobal.h>

namespace FcComponents
{

/// Line edit with clear button enabled by default and hint text support.
class FCComponentLibExport FcLineEdit : public QLineEdit
{
    Q_OBJECT
    Q_PROPERTY(bool clearButtonEnabled READ isClearButtonEnabled WRITE setClearButtonEnabled)
    Q_PROPERTY(QString hintText READ hintText WRITE setHintText)

public:
    explicit FcLineEdit(QWidget* parent = nullptr);
    explicit FcLineEdit(const QString& text, QWidget* parent = nullptr);

    QString hintText() const;
    void setHintText(const QString& hint);
};

}  // namespace FcComponents
