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

#include <QHBoxLayout>
#include <QPushButton>

#include "WizardFooter.h"
#include "FirstStartWidget.h"

using namespace StartGui;

WizardFooter::WizardFooter(int totalSteps, QWidget* parent)
    : QWidget(parent)
    , _totalSteps(totalSteps)
{
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    _skipButton = new QPushButton(this);
    _skipButton->setFlat(true);
    _skipButton->setCursor(Qt::PointingHandCursor);
    connect(_skipButton, &QPushButton::clicked, this, &WizardFooter::skipClicked);

    _backButton = new QPushButton(this);
    _backButton->setCursor(Qt::PointingHandCursor);
    _backButton->setMinimumWidth(100);
    connect(_backButton, &QPushButton::clicked, this, &WizardFooter::backClicked);

    _nextButton = new QPushButton(this);
    _nextButton->setCursor(Qt::PointingHandCursor);
    _nextButton->setMinimumWidth(120);
    connect(_nextButton, &QPushButton::clicked, this, &WizardFooter::nextClicked);

    applyThemeColors();

    layout->addWidget(_skipButton);
    layout->addStretch();
    layout->addWidget(_backButton);
    layout->addSpacing(8);
    layout->addWidget(_nextButton);

    retranslateUi();
    setStep(0);
}

void WizardFooter::setStep(int step)
{
    _currentStep = step;
    _backButton->setVisible(step > 0);

    bool isLastStep = (step >= _totalSteps - 1);
    if (isLastStep) {
        _nextButton->setText(tr("Start FreeCAD"));
    }
    else {
        _nextButton->setText(tr("Next"));
    }
}

void WizardFooter::retranslateUi()
{
    _skipButton->setText(tr("Skip setup"));
    _backButton->setText(tr("Back"));
    setStep(_currentStep);
}

void WizardFooter::applyThemeColors()
{
    bool dark = isWizardDarkMode();
    _skipButton->setStyleSheet(QString(
        "background: transparent; border: none; color: %1; padding: 8px 0;"
    ).arg(dark ? "#aaa" : "#666"));

    _backButton->setStyleSheet(QString(
        "background-color: transparent; border: 1px solid %1;"
        "border-radius: 8px; padding: 8px 24px; color: %2;"
    ).arg(dark ? "#555" : "#d0d0d0", dark ? "#e0e0e0" : "#333"));

    _nextButton->setStyleSheet(QStringLiteral(
        "background-color: #418FDE; color: white; border: none;"
        "border-radius: 8px; padding: 8px 24px; font-weight: bold;"
    ));
}
