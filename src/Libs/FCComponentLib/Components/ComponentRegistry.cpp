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

#include "ComponentRegistry.h"

#include <algorithm>
#include <cstring>
#include <set>

namespace FcComponents
{

ComponentRegistry& ComponentRegistry::instance()
{
    static ComponentRegistry registry;
    return registry;
}

void ComponentRegistry::add(const ComponentInfo& info)
{
    m_components.push_back(info);
}

const std::vector<ComponentInfo>& ComponentRegistry::all() const
{
    return m_components;
}

QWidget* ComponentRegistry::create(const char* name, QWidget* parent) const
{
    for (const auto& info : m_components) {
        if (std::strcmp(info.name, name) == 0) {
            return info.factory(parent);
        }
    }
    return nullptr;
}

std::vector<ComponentInfo> ComponentRegistry::byCategory(const char* category) const
{
    std::vector<ComponentInfo> result;
    for (const auto& info : m_components) {
        if (std::strcmp(info.category, category) == 0) {
            result.push_back(info);
        }
    }
    return result;
}

std::vector<std::string> ComponentRegistry::categories() const
{
    std::set<std::string> unique;
    for (const auto& info : m_components) {
        unique.insert(info.category);
    }
    return {unique.begin(), unique.end()};
}

}  // namespace FcComponents
