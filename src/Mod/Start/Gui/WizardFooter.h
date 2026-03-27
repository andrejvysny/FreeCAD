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

class QPushButton;

namespace StartGui
{

/// Footer bar for the setup wizard with Skip/Back/Next navigation.
/// Emits signals for each button. Updates button visibility and text per step.
class WizardFooter: public QWidget
{
    Q_OBJECT

public:
    explicit WizardFooter(int totalSteps, QWidget* parent = nullptr);

    void setStep(int step);
    void setSkipText(const QString& text);
    void retranslateUi();
    void applyThemeColors();

Q_SIGNALS:
    void skipClicked();
    void backClicked();
    void nextClicked();

private:
    int _totalSteps;
    int _currentStep = 0;
    QPushButton* _skipButton = nullptr;
    QPushButton* _backButton = nullptr;
    QPushButton* _nextButton = nullptr;
};

}  // namespace StartGui
