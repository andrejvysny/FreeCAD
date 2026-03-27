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

class QLabel;
class QPaintEvent;
class QStackedWidget;

namespace StartGui
{

/// Returns true if the current FreeCAD theme is dark
bool isWizardDarkMode();

class StepIndicatorWidget;
class WizardFooter;
class WizardBasicsPage;
class WizardWorkflowPage;
class WizardCommunityPage;
class WizardSummaryPage;

/// 4-step interactive setup wizard shown on first launch.
/// Contains WizardBasicsPage, WizardWorkflowPage, WizardCommunityPage, and WizardSummaryPage.
class FirstStartWidget: public QWidget
{
    Q_OBJECT

public:
    explicit FirstStartWidget(QWidget* parent = nullptr);
    bool eventFilter(QObject* object, QEvent* event) override;

    /// Reset wizard to step 0 (called on reopen from footer)
    void resetToFirstStep();

    Q_SIGNAL void dismissed();

    void applyThemeColors();

protected:
    void paintEvent(QPaintEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void setupUi();
    void retranslateUi();
    void goToStep(int step);
    void updateHeader();

    static constexpr int totalSteps = 4;
    static constexpr int StepBasics = 0;
    static constexpr int StepWorkflow = 1;
    static constexpr int StepCommunity = 2;
    static constexpr int StepSummary = 3;
    int _currentStep = 0;

    // Header
    QLabel* _titleLabel = nullptr;
    QLabel* _subtitleLabel = nullptr;
    StepIndicatorWidget* _stepIndicator = nullptr;

    // Pages
    QStackedWidget* _stepsWidget = nullptr;
    WizardBasicsPage* _basicsPage = nullptr;
    WizardWorkflowPage* _workflowPage = nullptr;
    WizardCommunityPage* _communityPage = nullptr;
    WizardSummaryPage* _summaryPage = nullptr;

    // Footer
    WizardFooter* _footer = nullptr;
};

}  // namespace StartGui
