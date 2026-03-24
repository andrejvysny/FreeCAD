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

class QLabel;

namespace StartGui
{

/// Step progress indicator showing colored dots.
/// Completed steps = green, current step = blue, future steps = gray.
class StepIndicatorWidget: public QWidget
{
    Q_OBJECT

public:
    explicit StepIndicatorWidget(int stepCount, QWidget* parent = nullptr);

    void setCurrentStep(int step);
    int currentStep() const;
    void applyThemeColors();

private:
    void updateDots();

    int _stepCount;
    int _currentStep = 0;
    std::vector<QLabel*> _dots;
};

}  // namespace StartGui
