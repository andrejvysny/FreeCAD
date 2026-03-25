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

#include <functional>

#include <QMetaObject>
#include <QWidget>

#include <FCComponentLib/FCComponentLibGlobal.h>

namespace FcComponents
{

/// Metadata describing a registered component for introspection and factory use.
struct FCComponentLibExport ComponentInfo
{
    const char* name;               ///< Class name, e.g. "FcSpinBox"
    const char* displayName;        ///< Human-readable name, e.g. "Spin Box"
    const char* category;           ///< Grouping category, e.g. "Inputs"
    const char* description;        ///< Brief description of the component
    const QMetaObject* metaObject;  ///< Qt meta-object for Q_PROPERTY introspection
    std::function<QWidget*(QWidget*)> factory;  ///< Factory function to create an instance
};

}  // namespace FcComponents


/// Register a component class with the global ComponentRegistry.
///
/// Creates a file-scope static registrar whose constructor runs before main(),
/// adding the component's metadata to the registry. The displayName is derived
/// by stripping the "Fc" prefix from the class name.
///
/// Usage (in .cpp file, at file scope):
///   FC_REGISTER_COMPONENT(FcSpinBox, "Inputs", "Integer spinbox with scroll guard")
// clang-format off
#define FC_REGISTER_COMPONENT(Class, Category, Desc)                                    \
    namespace                                                                           \
    {                                                                                   \
    struct Class##Registrar                                                             \
    {                                                                                   \
        Class##Registrar()                                                              \
        {                                                                               \
            FcComponents::ComponentInfo info {};                                        \
            info.name = #Class;                                                         \
            info.displayName = &(#Class)[2];                                             \
            info.category = Category;                                                   \
            info.description = Desc;                                                    \
            info.metaObject = &FcComponents::Class::staticMetaObject;                   \
            info.factory = [](QWidget* parent) -> QWidget* {                            \
                return new FcComponents::Class(parent);                                 \
            };                                                                          \
            FcComponents::ComponentRegistry::instance().add(info);                      \
        }                                                                               \
    };                                                                                  \
    static Class##Registrar s_##Class##Registrar;                                       \
    }  // namespace
// clang-format on
