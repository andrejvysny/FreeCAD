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

class QComboBox;
class QLabel;

namespace StartGui
{

class NavigationCardsWidget;
class SelectionCard;

/// Step 1 of the setup wizard: Language, Units, Navigation Style, Theme.
class WizardBasicsPage: public QWidget
{
    Q_OBJECT

public:
    explicit WizardBasicsPage(QWidget* parent = nullptr);
    void retranslateUi();

private:
    void createLanguageComboBox();
    void createUnitSystemComboBox();
    void createThemeCards();
    void preselectTheme();

    void onLanguageChanged(int index);
    void onUnitSystemChanged(int index);
    void onThemeCardClicked(int themeIndex);

    QLabel* _languageLabel = nullptr;
    QLabel* _unitsLabel = nullptr;
    QLabel* _themeLabel = nullptr;
    QComboBox* _languageComboBox = nullptr;
    QComboBox* _unitSystemComboBox = nullptr;
    NavigationCardsWidget* _navigationCards = nullptr;
    SelectionCard* _lightCard = nullptr;
    SelectionCard* _darkCard = nullptr;
};

}  // namespace StartGui
