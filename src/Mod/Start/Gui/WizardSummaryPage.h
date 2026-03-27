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
#include <array>

class QLabel;

namespace StartGui
{

/// Step 3 of the setup wizard: Summary of all chosen settings.
/// Displays a 3x2 grid of label+value pairs read from current preferences.
class WizardSummaryPage: public QWidget
{
    Q_OBJECT

public:
    explicit WizardSummaryPage(QWidget* parent = nullptr);
    void refreshSummary();
    void retranslateUi();
    void applyThemeColors();

private:
    QLabel* _headingLabel = nullptr;
    QLabel* _descriptionLabel = nullptr;
    QLabel* _checkmarkLabel = nullptr;

    // Summary grid: 6 cells, each with a category label + value label
    static constexpr int numCells = 6;
    std::array<QLabel*, numCells> _categoryLabels {};
    std::array<QLabel*, numCells> _valueLabels {};

    QLabel* _telemetryStatusLabel = nullptr;
};

}  // namespace StartGui
