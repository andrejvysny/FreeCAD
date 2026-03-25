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

#if defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(__CYGWIN__)
#  define FCCOMPONENTLIB_DECL_EXPORT __declspec(dllexport)
#  define FCCOMPONENTLIB_DECL_IMPORT __declspec(dllimport)
#else
#  define FCCOMPONENTLIB_DECL_EXPORT
#  define FCCOMPONENTLIB_DECL_IMPORT
#endif

#ifdef FCComponentLib_EXPORTS
#  define FCComponentLibExport FCCOMPONENTLIB_DECL_EXPORT
#else
#  define FCComponentLibExport FCCOMPONENTLIB_DECL_IMPORT
#endif
