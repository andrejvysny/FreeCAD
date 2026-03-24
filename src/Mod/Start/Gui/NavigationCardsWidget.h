// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
 *   Copyright (c) 2025 The FreeCAD Project Association AISBL               *
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

#include <QWidget>
#include <vector>

class QComboBox;
class QLabel;

namespace StartGui
{

class SelectionCard;

/// Shows 3 featured navigation style cards (CAD, Blender, Touchpad)
/// plus an "Other styles..." dropdown for remaining styles.
/// Cards and dropdown are mutually exclusive.
class NavigationCardsWidget: public QWidget
{
    Q_OBJECT

public:
    explicit NavigationCardsWidget(QWidget* parent = nullptr);
    void retranslateUi();

private:
    void onCardClicked(int cardIndex);
    void onDropdownChanged(int comboIndex);
    void applyNavigationStyle(const QByteArray& styleName);
    void preselectCurrentStyle();
    QWidget* createMappingWidget(const char* className);

    QLabel* _titleLabel = nullptr;
    QLabel* _subtitleLabel = nullptr;
    std::vector<SelectionCard*> _cards;
    std::vector<QByteArray> _cardStyleNames;
    QComboBox* _otherStylesCombo = nullptr;
};

}  // namespace StartGui
