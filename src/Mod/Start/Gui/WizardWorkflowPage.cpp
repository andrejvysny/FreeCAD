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
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QSizePolicy>
#include <QVBoxLayout>

#include "WizardWorkflowPage.h"
#include "SelectionCard.h"

#include <App/Application.h>
#include <Base/Parameter.h>

using namespace StartGui;

WizardWorkflowPage::WizardWorkflowPage(QWidget* parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    // Experience level section
    _experienceLabel = new QLabel(this);
    QFont boldFont = _experienceLabel->font();
    boldFont.setBold(true);
    _experienceLabel->setFont(boldFont);
    layout->addWidget(_experienceLabel);

    auto experienceRow = new QHBoxLayout();
    experienceRow->setSpacing(12);

    SelectionCard::Config beginnerCfg;
    beginnerCfg.title = tr("Beginner");
    beginnerCfg.description = tr("New to CAD. Show tooltips, simplified toolbars, and onboarding guides.");
    beginnerCfg.icon = QIcon(QStringLiteral(":/icons/icon-beginner.svg"));
    beginnerCfg.iconSize = {32, 32};
    _beginnerCard = new SelectionCard(beginnerCfg, this);

    SelectionCard::Config experiencedCfg;
    experiencedCfg.title = tr("Experienced");
    experiencedCfg.description = tr("Used SolidWorks, Fusion 360, or similar. Give me the full interface.");
    experiencedCfg.icon = QIcon(QStringLiteral(":/icons/icon-experienced.svg"));
    experiencedCfg.iconSize = {32, 32};
    _experiencedCard = new SelectionCard(experiencedCfg, this);

    connect(_beginnerCard, &SelectionCard::clicked, this, [this]() { onExperienceClicked(0); });
    connect(_experiencedCard, &SelectionCard::clicked, this, [this]() { onExperienceClicked(1); });

    _beginnerCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    _experiencedCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    experienceRow->addWidget(_beginnerCard);
    experienceRow->addWidget(_experiencedCard);
    layout->addLayout(experienceRow);

    // Layout profile section
    _layoutLabel = new QLabel(this);
    _layoutLabel->setFont(boldFont);
    layout->addWidget(_layoutLabel);

    auto layoutRow = new QHBoxLayout();
    layoutRow->setSpacing(12);

    SelectionCard::Config modernCfg;
    modernCfg.title = tr("Modern");
    modernCfg.description = tr("Organized toolbars, viewport status bar, cleaner panels.");
    modernCfg.icon = QIcon(QStringLiteral(":/thumbnails/layout-modern.svg"));
    modernCfg.iconSize = {160, 100};
    modernCfg.badge = tr("Recommended");
    _modernCard = new SelectionCard(modernCfg, this);

    SelectionCard::Config classicCfg;
    classicCfg.title = tr("Classic");
    classicCfg.description = tr("Traditional FreeCAD layout. All toolbars at top.");
    classicCfg.icon = QIcon(QStringLiteral(":/thumbnails/layout-classic.svg"));
    classicCfg.iconSize = {160, 100};
    _classicCard = new SelectionCard(classicCfg, this);

    connect(_modernCard, &SelectionCard::clicked, this, [this]() { onLayoutClicked(0); });
    connect(_classicCard, &SelectionCard::clicked, this, [this]() { onLayoutClicked(1); });

    _modernCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    _classicCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    layoutRow->addWidget(_modernCard);
    layoutRow->addWidget(_classicCard);
    layout->addLayout(layoutRow);

    // Default selections
    _beginnerCard->setChecked(true);
    _modernCard->setChecked(true);

    retranslateUi();
}

void WizardWorkflowPage::onExperienceClicked(int index)
{
    _beginnerCard->setChecked(index == 0);
    _experiencedCard->setChecked(index == 1);

    ParameterGrp::handle hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Mod/Start"
    );
    hGrp->SetASCII("ExperienceLevel", index == 0 ? "Beginner" : "Experienced");
}

void WizardWorkflowPage::onLayoutClicked(int index)
{
    _modernCard->setChecked(index == 0);
    _classicCard->setChecked(index == 1);

    ParameterGrp::handle hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Mod/Start"
    );
    hGrp->SetASCII("LayoutProfile", index == 0 ? "Modern" : "Classic");
}

void WizardWorkflowPage::retranslateUi()
{
    _experienceLabel->setText(tr("What's your CAD experience?"));
    _layoutLabel->setText(tr("Layout profile"));

    _beginnerCard->setTitle(tr("Beginner"));
    _beginnerCard->setDescription(
        tr("New to CAD. Show tooltips, simplified toolbars, and onboarding guides.")
    );
    _experiencedCard->setTitle(tr("Experienced"));
    _experiencedCard->setDescription(
        tr("Used SolidWorks, Fusion 360, or similar. Give me the full interface.")
    );
    _modernCard->setTitle(tr("Modern"));
    _modernCard->setDescription(
        tr("Organized toolbars, viewport status bar, cleaner panels.")
    );
    _modernCard->setBadge(tr("Recommended"));
    _classicCard->setTitle(tr("Classic"));
    _classicCard->setDescription(
        tr("Traditional FreeCAD layout. All toolbars at top.")
    );
}
