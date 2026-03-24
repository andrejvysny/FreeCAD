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

#include <algorithm>
#include <QHBoxLayout>
#include <QLabel>

#include "StepIndicatorWidget.h"
#include "FirstStartWidget.h"

using namespace StartGui;

static constexpr int dotSize = 10;
static constexpr int lineWidth = 24;

StepIndicatorWidget::StepIndicatorWidget(int stepCount, QWidget* parent)
    : QWidget(parent)
    , _stepCount(stepCount)
{
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    layout->setAlignment(Qt::AlignLeft);

    for (int i = 0; i < _stepCount; ++i) {
        if (i > 0) {
            // Connector line between dots
            auto line = new QLabel(this);
            line->setFixedSize(lineWidth, 3);
            line->setStyleSheet(QStringLiteral("background-color: #bbb; border: none;"));
            layout->addWidget(line);
        }

        auto dot = new QLabel(this);
        dot->setFixedSize(dotSize, dotSize);
        _dots.push_back(dot);
        layout->addWidget(dot);
    }

    updateDots();
}

void StepIndicatorWidget::setCurrentStep(int step)
{
    if (step == _currentStep || step < 0 || step >= _stepCount) {
        return;
    }
    _currentStep = step;
    updateDots();
}

int StepIndicatorWidget::currentStep() const
{
    return _currentStep;
}

void StepIndicatorWidget::updateDots()
{
    for (int i = 0; i < _stepCount; ++i) {
        auto* dot = _dots[static_cast<size_t>(i)];
        QString style;
        if (i < _currentStep) {
            style = QStringLiteral(
                "background-color: #4CAF50; border-radius: 5px; border: none;"
            );
        }
        else if (i == _currentStep) {
            style = QStringLiteral(
                "background-color: #418FDE; border-radius: 5px; border: none;"
            );
        }
        else {
            bool dark = isWizardDarkMode();
            style = QString(
                "background-color: transparent; border-radius: 5px; border: 2px solid %1;"
            ).arg(dark ? "#666" : "#bbb");
        }
        dot->setStyleSheet(style);
    }
}

void StepIndicatorWidget::applyThemeColors()
{
    // Update connector lines
    bool dark = isWizardDarkMode();
    auto lines = findChildren<QLabel*>();
    for (auto* line : lines) {
        if (std::find(_dots.begin(), _dots.end(), line) == _dots.end()) {
            // It's a connector line, not a dot
            line->setStyleSheet(QString("background-color: %1; border: none;")
                .arg(dark ? "#555" : "#bbb"));
        }
    }
    updateDots();
}
