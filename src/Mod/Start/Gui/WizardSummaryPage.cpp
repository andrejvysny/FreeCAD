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

#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

#include "WizardSummaryPage.h"
#include "FirstStartWidget.h"

#include <App/Application.h>
#include <Base/Parameter.h>
#include <Base/UnitsApi.h>
#include <Gui/Language/Translator.h>
#include <Gui/Navigation/NavigationStyle.h>

using namespace StartGui;

WizardSummaryPage::WizardSummaryPage(QWidget* parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);
    layout->setAlignment(Qt::AlignHCenter);

    // Checkmark icon
    _checkmarkLabel = new QLabel(this);
    _checkmarkLabel->setObjectName(QStringLiteral("wizardCheckmark"));
    _checkmarkLabel->setAlignment(Qt::AlignCenter);
    _checkmarkLabel->setFixedSize(64, 64);
    QPixmap checkPixmap(QStringLiteral(":/icons/checkmark-circle.svg"));
    if (!checkPixmap.isNull()) {
        _checkmarkLabel->setPixmap(checkPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    else {
        QFont checkFont = _checkmarkLabel->font();
        checkFont.setPointSize(28);
        _checkmarkLabel->setFont(checkFont);
        _checkmarkLabel->setText(QStringLiteral("\u2713"));
    }
    layout->addWidget(_checkmarkLabel, 0, Qt::AlignCenter);

    // "Ready to design" heading
    _headingLabel = new QLabel(this);
    _headingLabel->setObjectName(QStringLiteral("wizardSummaryHeading"));
    QFont headingFont = _headingLabel->font();
    headingFont.setPointSize(16);
    headingFont.setBold(true);
    _headingLabel->setFont(headingFont);
    _headingLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(_headingLabel);

    // Description
    _descriptionLabel = new QLabel(this);
    _descriptionLabel->setAlignment(Qt::AlignCenter);
    _descriptionLabel->setWordWrap(true);
    layout->addWidget(_descriptionLabel);

    layout->addSpacing(8);

    // 3x2 summary grid
    auto grid = new QGridLayout();
    grid->setSpacing(8);

    for (int i = 0; i < numCells; ++i) {
        auto cell = new QFrame(this);
        cell->setObjectName(QStringLiteral("summaryCell"));
        cell->setStyleSheet(QStringLiteral(
            "QFrame#summaryCell {"
            "  border-radius: 8px; padding: 8px 16px;"
            "  background-color: #e8e8e8; border: none;"
            "}"
        ));
        auto cellLayout = new QVBoxLayout(cell);
        cellLayout->setContentsMargins(12, 8, 12, 8);
        cellLayout->setSpacing(2);

        _categoryLabels[static_cast<size_t>(i)] = new QLabel(cell);
        _categoryLabels[static_cast<size_t>(i)]->setObjectName(QStringLiteral("summaryCategoryLabel"));
        QFont catFont = _categoryLabels[static_cast<size_t>(i)]->font();
        catFont.setPointSize(9);
        catFont.setCapitalization(QFont::AllUppercase);
        _categoryLabels[static_cast<size_t>(i)]->setFont(catFont);
        _categoryLabels[static_cast<size_t>(i)]->setAlignment(Qt::AlignCenter);
        cellLayout->addWidget(_categoryLabels[static_cast<size_t>(i)]);

        _valueLabels[static_cast<size_t>(i)] = new QLabel(cell);
        _valueLabels[static_cast<size_t>(i)]->setObjectName(QStringLiteral("summaryValueLabel"));
        QFont valFont = _valueLabels[static_cast<size_t>(i)]->font();
        valFont.setBold(true);
        _valueLabels[static_cast<size_t>(i)]->setFont(valFont);
        _valueLabels[static_cast<size_t>(i)]->setAlignment(Qt::AlignCenter);
        cellLayout->addWidget(_valueLabels[static_cast<size_t>(i)]);

        int row = i / 3;
        int col = i % 3;
        grid->addWidget(cell, row, col);
    }

    layout->addLayout(grid);

    // Telemetry status (hidden by default, shown if user opted in)
    _telemetryStatusLabel = new QLabel(this);
    _telemetryStatusLabel->setObjectName(QStringLiteral("telemetryStatus"));
    _telemetryStatusLabel->setAlignment(Qt::AlignCenter);
    _telemetryStatusLabel->setWordWrap(true);
    _telemetryStatusLabel->setVisible(false);
    layout->addWidget(_telemetryStatusLabel);

    retranslateUi();
}

void WizardSummaryPage::refreshSummary()
{
    // Language
    auto langStr = Gui::Translator::instance()->activeLanguage();
    _valueLabels[0]->setText(QString::fromStdString(langStr));

    // Units
    auto unitDescriptions = Base::UnitsApi::getDescriptions();
    ParameterGrp::handle hGrpUnits = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Units"
    );
    auto schema = hGrpUnits->GetInt("UserSchema", 0);
    if (schema >= 0 && schema < static_cast<int>(unitDescriptions.size())) {
        _valueLabels[1]->setText(QString::fromStdString(unitDescriptions[static_cast<size_t>(schema)]));
    }

    // Navigation
    ParameterGrp::handle hGrpView = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/View"
    );
    auto navName = hGrpView->GetASCII(
        "NavigationStyle",
        Gui::CADNavigationStyle::getClassTypeId().getName()
    );
    // Get friendly name
    std::map<Base::Type, std::string> styles = Gui::UserNavigationStyle::getUserFriendlyNames();
    Base::Type navType = Base::Type::fromName(navName.c_str());
    auto it = styles.find(navType);
    if (it != styles.end()) {
        _valueLabels[2]->setText(QString::fromStdString(it->second));
    }
    else {
        _valueLabels[2]->setText(QString::fromStdString(navName));
    }

    // Theme
    ParameterGrp::handle hGrpMain = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/MainWindow"
    );
    auto styleSheet = QString::fromStdString(hGrpMain->GetASCII("StyleSheet", ""));
    if (styleSheet.contains(QLatin1String("Dark"), Qt::CaseInsensitive)) {
        _valueLabels[3]->setText(tr("Dark"));
    }
    else {
        _valueLabels[3]->setText(tr("Light"));
    }

    // Experience
    ParameterGrp::handle hGrpStart = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Mod/Start"
    );
    auto experience = QString::fromStdString(hGrpStart->GetASCII("ExperienceLevel", "Beginner"));
    _valueLabels[4]->setText(experience);

    // Layout
    auto layoutProfile = QString::fromStdString(hGrpStart->GetASCII("LayoutProfile", "Modern"));
    _valueLabels[5]->setText(layoutProfile);

    // Telemetry status
    bool telemetryOn = hGrpStart->GetBool("TelemetryOptIn", false);
    if (telemetryOn) {
        _telemetryStatusLabel->setText(
            QStringLiteral("\u2713 ")
            + tr("Usage statistics enabled \u2014 thank you for contributing."));
        _telemetryStatusLabel->setStyleSheet(
            QStringLiteral("color: #4CAF50; border: none;"));
        _telemetryStatusLabel->setVisible(true);
    }
    else {
        _telemetryStatusLabel->setVisible(false);
    }
}

void WizardSummaryPage::retranslateUi()
{
    _headingLabel->setText(tr("Ready to design"));
    _descriptionLabel->setText(
        tr("FreeCAD is configured for your workflow.") + QLatin1String(" ")
        + tr("Start by creating a new part or opening an example project.")
    );

    _categoryLabels[0]->setText(tr("Language"));
    _categoryLabels[1]->setText(tr("Units"));
    _categoryLabels[2]->setText(tr("Navigation"));
    _categoryLabels[3]->setText(tr("Theme"));
    _categoryLabels[4]->setText(tr("Experience"));
    _categoryLabels[5]->setText(tr("Layout"));
}

void WizardSummaryPage::applyThemeColors()
{
    bool dark = isWizardDarkMode();
    QString cellBg = dark ? QStringLiteral("#444") : QStringLiteral("#e8e8e8");

    auto cells = findChildren<QFrame*>(QStringLiteral("summaryCell"));
    for (auto* cell : cells) {
        cell->setStyleSheet(QString(
            "QFrame#summaryCell {"
            "  border-radius: 8px; padding: 8px 16px;"
            "  background-color: %1; border: none;"
            "}"
        ).arg(cellBg));
    }
}
