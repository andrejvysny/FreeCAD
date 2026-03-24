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

class QLabel;

namespace StartGui
{

class SelectionCard;

/// Step 2 of the setup wizard: Experience level and layout profile selection.
/// Saves preferences but does not implement backend behavior (deferred).
class WizardWorkflowPage: public QWidget
{
    Q_OBJECT

public:
    explicit WizardWorkflowPage(QWidget* parent = nullptr);
    void retranslateUi();

private:
    void onExperienceClicked(int index);
    void onLayoutClicked(int index);

    QLabel* _experienceLabel = nullptr;
    QLabel* _layoutLabel = nullptr;
    SelectionCard* _beginnerCard = nullptr;
    SelectionCard* _experiencedCard = nullptr;
    SelectionCard* _modernCard = nullptr;
    SelectionCard* _classicCard = nullptr;
};

}  // namespace StartGui
