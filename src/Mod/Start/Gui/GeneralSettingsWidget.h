// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
 *   Copyright (c) 2024 The FreeCAD Project Association AISBL               *
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
#include <gsl/pointers>

class QLabel;
class QComboBox;

namespace StartGui
{

class GeneralSettingsWidget: public QWidget
{
    Q_OBJECT
public:
    explicit GeneralSettingsWidget(QWidget* parent = nullptr);
    void refreshFromPreferences();

    bool eventFilter(QObject* object, QEvent* event) override;

private:
    void retranslateUi();

    void setupUi();
    gsl::owner<QComboBox*> createLanguageComboBox();
    gsl::owner<QComboBox*> createUnitSystemComboBox();
    gsl::owner<QComboBox*> createNavigationStyleComboBox();
    gsl::owner<QComboBox*> createOrbitStyleComboBox();
    void populateUnitSystemComboBox();
    void populateNavigationStyleComboBox();
    void populateOrbitStyleComboBox();

    void onLanguageChanged(int index);
    void onUnitSystemChanged(int index);
    void onNavigationStyleChanged(int index);
    void onOrbitStyleChanged(int index);

    // Non-owning pointers to things that need to be re-translated when the language changes
    QLabel* _languageLabel;
    QLabel* _unitSystemLabel;
    QLabel* _navigationStyleLabel;
    QLabel* _orbitStyleLabel;
    QLabel* _languageHelperLabel;
    QLabel* _unitSystemHelperLabel;
    QLabel* _navigationStyleHelperLabel;
    QLabel* _orbitStyleHelperLabel;
    QComboBox* _languageComboBox;
    QComboBox* _unitSystemComboBox;
    QComboBox* _navigationStyleComboBox;
    QComboBox* _orbitStyleComboBox;
};

}  // namespace StartGui
