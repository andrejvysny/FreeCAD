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

#include <QFont>
#include <QGuiApplication>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "FirstStartWidget.h"
#include "SelectionCard.h"
#include "StepIndicatorWidget.h"
#include "WizardFooter.h"
#include "WizardBasicsPage.h"
#include "WizardWorkflowPage.h"
#include "WizardSummaryPage.h"

#include <App/Application.h>
#include <Base/Parameter.h>

using namespace StartGui;

bool StartGui::isWizardDarkMode()
{
    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/MainWindow"
    );
    auto styleSheet = QString::fromStdString(hGrp->GetASCII("StyleSheet", ""));
    return styleSheet.contains(QLatin1String("Dark"), Qt::CaseInsensitive);
}

FirstStartWidget::FirstStartWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QLatin1String("FirstStartWidget"));
    setupUi();
    qApp->installEventFilter(this);
}

void FirstStartWidget::setupUi()
{
    setMaximumWidth(750);

    auto outerLayout = new QVBoxLayout(this);
    outerLayout->setSpacing(12);
    outerLayout->setContentsMargins(32, 24, 32, 24);

    // Header: title + subtitle + step indicator
    _titleLabel = new QLabel(this);
    _titleLabel->setObjectName(QStringLiteral("wizardTitle"));
    QFont titleFont = _titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    _titleLabel->setFont(titleFont);
    outerLayout->addWidget(_titleLabel);

    _subtitleLabel = new QLabel(this);
    _subtitleLabel->setObjectName(QStringLiteral("wizardSubtitle"));
    _subtitleLabel->setWordWrap(true);
    outerLayout->addWidget(_subtitleLabel);

    _stepIndicator = new StepIndicatorWidget(totalSteps, this);
    outerLayout->addWidget(_stepIndicator);

    // Stacked pages
    _stepsWidget = new QStackedWidget(this);

    _basicsPage = new WizardBasicsPage(this);
    _stepsWidget->addWidget(_basicsPage);

    _workflowPage = new WizardWorkflowPage(this);
    _stepsWidget->addWidget(_workflowPage);

    _summaryPage = new WizardSummaryPage(this);
    _stepsWidget->addWidget(_summaryPage);

    outerLayout->addWidget(_stepsWidget);

    // Footer
    _footer = new WizardFooter(totalSteps, this);
    outerLayout->addWidget(_footer);

    connect(_footer, &WizardFooter::skipClicked, this, &FirstStartWidget::dismissed);
    connect(_footer, &WizardFooter::backClicked, this, [this]() {
        if (_currentStep > 0) {
            goToStep(_currentStep - 1);
        }
    });
    connect(_footer, &WizardFooter::nextClicked, this, [this]() {
        if (_currentStep < totalSteps - 1) {
            goToStep(_currentStep + 1);
        }
        else {
            Q_EMIT dismissed();
        }
    });

    applyThemeColors();
    goToStep(0);
    retranslateUi();
}

void FirstStartWidget::applyThemeColors()
{
    bool dark = isWizardDarkMode();
    QString textColor = dark ? QStringLiteral("#e0e0e0") : QStringLiteral("#333");
    QString comboBg = dark ? QStringLiteral("#383838") : QStringLiteral("white");
    QString comboBorder = dark ? QStringLiteral("#555") : QStringLiteral("#ccc");

    setStyleSheet(QString(
        "QWidget#FirstStartWidget * {"
        "  background-color: transparent;"
        "}"
        "QWidget#FirstStartWidget QLabel {"
        "  border: none;"
        "  color: %1;"
        "}"
        "QWidget#FirstStartWidget QComboBox {"
        "  background-color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 4px;"
        "  padding: 4px 8px;"
        "  color: %1;"
        "}"
    ).arg(textColor, comboBg, comboBorder));

    // Update child widgets (guard against null during construction)
    if (_stepIndicator) {
        _stepIndicator->applyThemeColors();
    }
    if (_footer) {
        _footer->applyThemeColors();
    }

    // Update all SelectionCards in all pages
    auto cards = findChildren<SelectionCard*>();
    for (auto* card : cards) {
        card->updateStyle();
    }

    // Update summary cells
    if (_summaryPage) {
        _summaryPage->applyThemeColors();
    }

    update();  // trigger paintEvent repaint
}

void FirstStartWidget::paintEvent(QPaintEvent* /*event*/)
{
    bool dark = isWizardDarkMode();
    QColor bg = dark ? QColor(45, 45, 45) : Qt::white;
    QColor border = dark ? QColor(85, 85, 85) : QColor(208, 208, 208);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(rect(), 16, 16);
    painter.fillPath(path, bg);

    painter.setPen(QPen(border, 1));
    painter.drawPath(path);
}

void FirstStartWidget::changeEvent(QEvent* event)
{
    // Note: do NOT call applyThemeColors() here — setStyleSheet() triggers StyleChange
    // which causes infinite recursion. Theme updates are triggered explicitly from
    // WizardBasicsPage::onThemeCardClicked() via QTimer::singleShot.
    QWidget::changeEvent(event);
}

void FirstStartWidget::goToStep(int step)
{
    _currentStep = step;
    _stepsWidget->setCurrentIndex(step);
    _stepIndicator->setCurrentStep(step);
    _footer->setStep(step);
    updateHeader();

    if (step == totalSteps - 1 && _summaryPage) {
        _summaryPage->refreshSummary();
    }
}

void FirstStartWidget::resetToFirstStep()
{
    goToStep(0);
}

void FirstStartWidget::updateHeader()
{
    QString application = QString::fromStdString(App::Application::getExecutableName());

    switch (_currentStep) {
        case 0:
            _titleLabel->setText(tr("Welcome to %1").arg(application));
            _subtitleLabel->setText(
                tr("Set your preferences below.") + QLatin1String(" ")
                + tr("You can change everything later in Edit > Preferences.")
            );
            break;
        case 1:
            _titleLabel->setText(tr("Your workflow"));
            _subtitleLabel->setText(
                tr("Help us tailor %1 to your experience level.").arg(application)
            );
            break;
        case 2:
            _titleLabel->setText(tr("You're all set"));
            _subtitleLabel->setText(
                tr("Here's a summary.") + QLatin1String(" ")
                + tr("Change anything later in Edit > Preferences.")
            );
            break;
    }
}

bool FirstStartWidget::eventFilter(QObject* object, QEvent* event)
{
    if (object == this && event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    return QWidget::eventFilter(object, event);
}

void FirstStartWidget::retranslateUi()
{
    updateHeader();
    _footer->retranslateUi();
    if (_basicsPage) {
        _basicsPage->retranslateUi();
    }
    if (_workflowPage) {
        _workflowPage->retranslateUi();
    }
    if (_summaryPage) {
        _summaryPage->retranslateUi();
    }
}
