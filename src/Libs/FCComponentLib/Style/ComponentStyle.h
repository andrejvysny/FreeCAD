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

#include <QProxyStyle>

#include <FCComponentLib/FCComponentLibGlobal.h>

namespace FcComponents::Tokens
{
class TokenManager;
}

namespace FcComponents
{

/// QProxyStyle("Fusion") driven by design tokens.
///
/// Applies resolved token colors to QPalette and overrides specific style
/// hints for consistent widget behavior across platforms.
class FCComponentLibExport ComponentStyle : public QProxyStyle
{
    Q_OBJECT

public:
    ComponentStyle();

    /// Apply token-derived palette to a QApplication.
    /// Reads color tokens from the manager and sets QPalette roles.
    static void apply(QApplication* app, const Tokens::TokenManager& manager);

    /// Generate a QPalette from resolved tokens.
    static QPalette paletteFromTokens(const Tokens::TokenManager& manager);

protected:
    int styleHint(StyleHint hint,
                  const QStyleOption* option = nullptr,
                  const QWidget* widget = nullptr,
                  QStyleHintReturn* returnData = nullptr) const override;
};

}  // namespace FcComponents
