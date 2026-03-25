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

#include <string>
#include <vector>

#include <QWidget>

#include <FCComponentLib/FCComponentLibGlobal.h>
#include <FCComponentLib/Components/ComponentMeta.h>

namespace FcComponents
{

/// Singleton registry of all FCComponentLib widgets.
///
/// Components self-register via the FC_REGISTER_COMPONENT macro at static
/// initialization time. The registry provides lookup by name and category,
/// and a factory method to instantiate widgets by name.
class FCComponentLibExport ComponentRegistry
{
public:
    /// Returns the singleton instance.
    static ComponentRegistry& instance();

    /// Register a component. Called automatically by FC_REGISTER_COMPONENT.
    void add(const ComponentInfo& info);

    /// Returns all registered components.
    const std::vector<ComponentInfo>& all() const;

    /// Create a widget by its registered name, or nullptr if not found.
    QWidget* create(const char* name, QWidget* parent = nullptr) const;

    /// Returns all components in the given category.
    std::vector<ComponentInfo> byCategory(const char* category) const;

    /// Returns a sorted list of unique category names.
    std::vector<std::string> categories() const;

    ComponentRegistry(const ComponentRegistry&) = delete;
    ComponentRegistry& operator=(const ComponentRegistry&) = delete;

private:
    ComponentRegistry() = default;

    std::vector<ComponentInfo> m_components;
};

}  // namespace FcComponents
