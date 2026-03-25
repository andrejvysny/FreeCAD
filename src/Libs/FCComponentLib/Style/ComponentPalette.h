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

#include <QPalette>

#include <FCComponentLib/FCComponentLibGlobal.h>

namespace FcComponents
{

/// Utility class for generating QPalette presets without the full token system.
///
/// Provides built-in dark and light palettes for quick theme switching
/// in the gallery app when YAML token files are not available.
class FCComponentLibExport ComponentPalette
{
public:
    /// Returns a dark palette matching FreeCAD's default dark theme.
    static QPalette darkPalette();

    /// Returns a light palette matching FreeCAD's default light theme.
    static QPalette lightPalette();
};

}  // namespace FcComponents
