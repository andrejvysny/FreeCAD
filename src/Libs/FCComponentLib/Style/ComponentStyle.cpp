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

#include "ComponentStyle.h"

#include <QApplication>
#include <QPalette>

#include <Base/Color.h>
#include <FCComponentLib/Tokens/TokenManager.h>

namespace FcComponents
{

namespace
{

QColor toQColor(const Base::Color& c)
{
    return QColor::fromRgbF(c.r, c.g, c.b, c.a);
}

/// Try to resolve a color token, return fallback if not found.
QColor resolveColor(const Tokens::TokenManager& manager, const std::string& name,
                    const QColor& fallback)
{
    auto value = manager.resolve(name);
    if (value && value->holds<Base::Color>()) {
        return toQColor(value->get<Base::Color>());
    }
    return fallback;
}

}  // namespace

ComponentStyle::ComponentStyle()
    : QProxyStyle(QStringLiteral("Fusion"))
{
}

void ComponentStyle::apply(QApplication* app, const Tokens::TokenManager& manager)
{
    QPalette pal = paletteFromTokens(manager);
    app->setPalette(pal);
}

QPalette ComponentStyle::paletteFromTokens(const Tokens::TokenManager& manager)
{
    QPalette pal;

    // Map token names to QPalette roles
    QColor window = resolveColor(manager, "GeneralBackgroundColor", pal.color(QPalette::Window));
    QColor windowText = resolveColor(manager, "TextForegroundColor", pal.color(QPalette::WindowText));
    QColor base = resolveColor(manager, "TextEditFieldBackgroundColor", pal.color(QPalette::Base));
    QColor altBase = resolveColor(
        manager, "GeneralAlternateBackgroundColor", pal.color(QPalette::AlternateBase));
    QColor button = resolveColor(
        manager, "ButtonTopBackgroundColor", pal.color(QPalette::Button));
    QColor buttonText = resolveColor(manager, "TextForegroundColor", pal.color(QPalette::ButtonText));
    QColor highlight = resolveColor(manager, "AccentColor", pal.color(QPalette::Highlight));
    QColor highlightText = resolveColor(
        manager, "TextForegroundColor", pal.color(QPalette::HighlightedText));
    QColor link = resolveColor(manager, "TextUrlColor", pal.color(QPalette::Link));
    QColor disabledText = resolveColor(
        manager, "TextDisabledColor", pal.color(QPalette::Disabled, QPalette::WindowText));

    pal.setColor(QPalette::Window, window);
    pal.setColor(QPalette::WindowText, windowText);
    pal.setColor(QPalette::Base, base);
    pal.setColor(QPalette::AlternateBase, altBase);
    pal.setColor(QPalette::Button, button);
    pal.setColor(QPalette::ButtonText, buttonText);
    pal.setColor(QPalette::Highlight, highlight);
    pal.setColor(QPalette::HighlightedText, highlightText);
    pal.setColor(QPalette::Link, link);
    pal.setColor(QPalette::Disabled, QPalette::WindowText, disabledText);
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, disabledText);

    return pal;
}

int ComponentStyle::styleHint(StyleHint hint, const QStyleOption* option,
                              const QWidget* widget, QStyleHintReturn* returnData) const
{
    // Disable scroll-on-focus for spinboxes globally — individual widgets handle this
    if (hint == SH_SpinBox_StepModifier) {
        return Qt::NoModifier;
    }

    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

}  // namespace FcComponents
