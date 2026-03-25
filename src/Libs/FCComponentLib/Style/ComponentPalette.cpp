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

#include "ComponentPalette.h"

#include <QColor>

namespace FcComponents
{

namespace
{

/// Set a color role across Active, Inactive, and Disabled groups.
void setAllGroups(QPalette& pal, QPalette::ColorRole role, const QColor& color)
{
    pal.setColor(QPalette::Active, role, color);
    pal.setColor(QPalette::Inactive, role, color);
    pal.setColor(QPalette::Disabled, role, color);
}

}  // namespace

QPalette ComponentPalette::darkPalette()
{
    QPalette pal;

    // Core colors derived from FreeCAD Dark.yaml: PrimaryColor=#191919
    QColor window(0x2D, 0x2D, 0x2D);
    QColor windowText(0xFF, 0xFF, 0xFF);
    QColor base(0x19, 0x19, 0x19);
    QColor altBase(0x35, 0x35, 0x35);
    QColor text(0xFF, 0xFF, 0xFF);
    QColor button(0x42, 0x42, 0x42);
    QColor buttonText(0xFF, 0xFF, 0xFF);
    QColor brightText(0xFF, 0x00, 0x00);
    QColor highlight(0x30, 0x80, 0xD0);
    QColor highlightText(0xFF, 0xFF, 0xFF);
    QColor link(0x00, 0x95, 0xFF);
    QColor linkVisited(0xA0, 0x70, 0xFF);

    // Derived shading
    QColor light(0x50, 0x50, 0x50);
    QColor midlight(0x46, 0x46, 0x46);
    QColor dark(0x1A, 0x1A, 0x1A);
    QColor mid(0x3A, 0x3A, 0x3A);
    QColor shadow(0x0A, 0x0A, 0x0A);

    // Tooltip & placeholder
    QColor toolTipBase(0x42, 0x42, 0x42);
    QColor toolTipText(0xFF, 0xFF, 0xFF);
    QColor placeholderText(0x80, 0x80, 0x80);

    // Set all roles for Active + Inactive + Disabled
    setAllGroups(pal, QPalette::Window, window);
    setAllGroups(pal, QPalette::WindowText, windowText);
    setAllGroups(pal, QPalette::Base, base);
    setAllGroups(pal, QPalette::AlternateBase, altBase);
    setAllGroups(pal, QPalette::Text, text);
    setAllGroups(pal, QPalette::Button, button);
    setAllGroups(pal, QPalette::ButtonText, buttonText);
    setAllGroups(pal, QPalette::BrightText, brightText);
    setAllGroups(pal, QPalette::Light, light);
    setAllGroups(pal, QPalette::Midlight, midlight);
    setAllGroups(pal, QPalette::Dark, dark);
    setAllGroups(pal, QPalette::Mid, mid);
    setAllGroups(pal, QPalette::Shadow, shadow);
    setAllGroups(pal, QPalette::Highlight, highlight);
    setAllGroups(pal, QPalette::HighlightedText, highlightText);
    setAllGroups(pal, QPalette::Link, link);
    setAllGroups(pal, QPalette::LinkVisited, linkVisited);
    setAllGroups(pal, QPalette::ToolTipBase, toolTipBase);
    setAllGroups(pal, QPalette::ToolTipText, toolTipText);
    setAllGroups(pal, QPalette::PlaceholderText, placeholderText);

    // Disabled overrides — dimmed text
    QColor disabledText(0x68, 0x68, 0x68);
    pal.setColor(QPalette::Disabled, QPalette::WindowText, disabledText);
    pal.setColor(QPalette::Disabled, QPalette::Text, disabledText);
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, disabledText);
    pal.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledText);
    pal.setColor(QPalette::Disabled, QPalette::PlaceholderText, QColor(0x50, 0x50, 0x50));

    return pal;
}

QPalette ComponentPalette::lightPalette()
{
    QPalette pal;

    QColor window(0xF0, 0xF0, 0xF0);
    QColor windowText(0x1A, 0x1A, 0x1A);
    QColor base(0xFF, 0xFF, 0xFF);
    QColor altBase(0xE8, 0xE8, 0xE8);
    QColor text(0x1A, 0x1A, 0x1A);
    QColor button(0xE0, 0xE0, 0xE0);
    QColor buttonText(0x1A, 0x1A, 0x1A);
    QColor brightText(0xFF, 0x00, 0x00);
    QColor highlight(0x30, 0x80, 0xD0);
    QColor highlightText(0xFF, 0xFF, 0xFF);
    QColor link(0x00, 0x68, 0xC8);
    QColor linkVisited(0x80, 0x40, 0xC0);

    QColor light(0xFF, 0xFF, 0xFF);
    QColor midlight(0xE8, 0xE8, 0xE8);
    QColor dark(0xA0, 0xA0, 0xA0);
    QColor mid(0xC8, 0xC8, 0xC8);
    QColor shadow(0x60, 0x60, 0x60);

    QColor toolTipBase(0xFF, 0xFF, 0xDC);
    QColor toolTipText(0x1A, 0x1A, 0x1A);
    QColor placeholderText(0xA0, 0xA0, 0xA0);

    setAllGroups(pal, QPalette::Window, window);
    setAllGroups(pal, QPalette::WindowText, windowText);
    setAllGroups(pal, QPalette::Base, base);
    setAllGroups(pal, QPalette::AlternateBase, altBase);
    setAllGroups(pal, QPalette::Text, text);
    setAllGroups(pal, QPalette::Button, button);
    setAllGroups(pal, QPalette::ButtonText, buttonText);
    setAllGroups(pal, QPalette::BrightText, brightText);
    setAllGroups(pal, QPalette::Light, light);
    setAllGroups(pal, QPalette::Midlight, midlight);
    setAllGroups(pal, QPalette::Dark, dark);
    setAllGroups(pal, QPalette::Mid, mid);
    setAllGroups(pal, QPalette::Shadow, shadow);
    setAllGroups(pal, QPalette::Highlight, highlight);
    setAllGroups(pal, QPalette::HighlightedText, highlightText);
    setAllGroups(pal, QPalette::Link, link);
    setAllGroups(pal, QPalette::LinkVisited, linkVisited);
    setAllGroups(pal, QPalette::ToolTipBase, toolTipBase);
    setAllGroups(pal, QPalette::ToolTipText, toolTipText);
    setAllGroups(pal, QPalette::PlaceholderText, placeholderText);

    QColor disabledText(0xA0, 0xA0, 0xA0);
    pal.setColor(QPalette::Disabled, QPalette::WindowText, disabledText);
    pal.setColor(QPalette::Disabled, QPalette::Text, disabledText);
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, disabledText);
    pal.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledText);
    pal.setColor(QPalette::Disabled, QPalette::PlaceholderText, QColor(0xC0, 0xC0, 0xC0));

    return pal;
}

}  // namespace FcComponents
