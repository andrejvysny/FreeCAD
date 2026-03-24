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
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

#include "WizardCommunityPage.h"
#include "FirstStartWidget.h"

#include <App/Application.h>
#include <Base/Parameter.h>

using namespace StartGui;

WizardCommunityPage::WizardCommunityPage(QWidget* parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    layout->setAlignment(Qt::AlignHCenter);

    // Sparkle icon
    _iconLabel = new QLabel(this);
    _iconLabel->setObjectName(QStringLiteral("communityIcon"));
    _iconLabel->setAlignment(Qt::AlignCenter);
    _iconLabel->setFixedSize(64, 64);
    QPixmap iconPixmap(QStringLiteral(":/icons/icon-telemetry.svg"));
    if (!iconPixmap.isNull()) {
        _iconLabel->setPixmap(
            iconPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    else {
        QFont iconFont = _iconLabel->font();
        iconFont.setPointSize(28);
        _iconLabel->setFont(iconFont);
        _iconLabel->setText(QStringLiteral("\u2728"));
    }
    layout->addWidget(_iconLabel, 0, Qt::AlignCenter);

    // Heading
    _headingLabel = new QLabel(this);
    _headingLabel->setObjectName(QStringLiteral("communityHeading"));
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

    // 3x2 data grid showing what is/isn't collected
    auto grid = new QGridLayout();
    grid->setVerticalSpacing(4);
    grid->setHorizontalSpacing(24);
    grid->setContentsMargins(0, 0, 0, 0);

    for (int i = 0; i < numDataItems; ++i) {
        _dataItems[static_cast<size_t>(i)] = new QLabel(this);
        _dataItems[static_cast<size_t>(i)]->setTextFormat(Qt::RichText);
        _dataItems[static_cast<size_t>(i)]->setWordWrap(false);

        int row = i / 2;
        int col = i % 2;
        grid->addWidget(_dataItems[static_cast<size_t>(i)], row, col);
    }

    auto gridWrapper = new QWidget(this);
    gridWrapper->setLayout(grid);
    layout->addWidget(gridWrapper, 0, Qt::AlignHCenter);

    // Enable statistics button (toggle)
    _enableButton = new QPushButton(this);
    _enableButton->setCheckable(true);
    _enableButton->setCursor(Qt::PointingHandCursor);
    _enableButton->setMinimumWidth(200);
    connect(_enableButton, &QPushButton::clicked, this, &WizardCommunityPage::onEnableClicked);
    layout->addWidget(_enableButton, 0, Qt::AlignCenter);

    layout->addSpacing(4);

    // Info bar at bottom
    _infoBar = new QFrame(this);
    _infoBar->setObjectName(QStringLiteral("communityInfoBar"));
    auto infoLayout = new QHBoxLayout(_infoBar);
    infoLayout->setContentsMargins(16, 10, 16, 10);

    _infoBarLabel = new QLabel(_infoBar);
    _infoBarLabel->setWordWrap(true);
    _infoBarLabel->setAlignment(Qt::AlignCenter);
    infoLayout->addWidget(_infoBarLabel);

    layout->addWidget(_infoBar);

    updateButtonStyle();
    applyThemeColors();
    retranslateUi();
}

void WizardCommunityPage::onEnableClicked()
{
    _telemetryEnabled = _enableButton->isChecked();
    updateButtonStyle();
    retranslateUi();

    ParameterGrp::handle hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Mod/Start");
    hGrp->SetBool("TelemetryOptIn", _telemetryEnabled);
}

bool WizardCommunityPage::isTelemetryEnabled() const
{
    return _telemetryEnabled;
}

void WizardCommunityPage::updateButtonStyle()
{
    bool dark = isWizardDarkMode();

    if (_telemetryEnabled) {
        _enableButton->setStyleSheet(QStringLiteral(
            "background-color: #418FDE; color: white; border: none;"
            "border-radius: 8px; padding: 8px 24px; font-weight: bold;"));
    }
    else {
        QString bg = dark ? QStringLiteral("rgba(65,143,222,0.15)")
                          : QStringLiteral("rgba(65,143,222,0.08)");
        QString textColor = QStringLiteral("#418FDE");
        _enableButton->setStyleSheet(
            QString("background-color: %1; color: %2; border: 1px solid %2;"
                    "border-radius: 8px; padding: 8px 24px; font-weight: bold;")
                .arg(bg, textColor));
    }
}

void WizardCommunityPage::retranslateUi()
{
    _headingLabel->setText(tr("Help improve FreeCAD"));
    _descriptionLabel->setText(
        tr("Share anonymous usage statistics to help developers find bugs and "
           "prioritize features. This is completely optional and installed as an addon."));

    // Data items: 0-3 are included (green check), 4-5 are excluded (red cross + strikethrough)
    bool dark = isWizardDarkMode();
    QString mutedColor = dark ? QStringLiteral("#888") : QStringLiteral("#999");

    struct DataItemInfo
    {
        QString text;
        bool included;
    };
    DataItemInfo items[] = {
        {tr("Workbench usage frequency"), true},
        {tr("Crash reports & error logs"), true},
        {tr("Feature usage patterns"), true},
        {tr("Hardware & OS info"), true},
        {tr("File contents or designs"), false},
        {tr("Personal information"), false},
    };

    for (int i = 0; i < numDataItems; ++i) {
        const auto& item = items[i];
        QString html;
        if (item.included) {
            html = QStringLiteral("<span style=\"color:#4CAF50; font-size:14px;\">\u2714</span>"
                                  "&nbsp;&nbsp;%1")
                       .arg(item.text);
        }
        else {
            html = QStringLiteral(
                       "<span style=\"color:#e53935; font-size:14px;\">\u2718</span>"
                       "&nbsp;&nbsp;<span style=\"text-decoration:line-through; color:%1;\">%2</span>")
                       .arg(mutedColor, item.text);
        }
        _dataItems[static_cast<size_t>(i)]->setText(html);
    }

    // Button text depends on toggle state
    if (_telemetryEnabled) {
        _enableButton->setText(tr("Statistics enabled \u2713"));
    }
    else {
        _enableButton->setText(tr("Enable statistics"));
    }

    _infoBarLabel->setText(
        QStringLiteral("\u2611 ")
        + tr("Installed as addon \u2014 can be enabled or removed anytime in Addon Manager"));
}

void WizardCommunityPage::applyThemeColors()
{
    bool dark = isWizardDarkMode();
    QString barBg = dark ? QStringLiteral("#444") : QStringLiteral("#e8e8e8");

    _infoBar->setStyleSheet(
        QString("QFrame#communityInfoBar {"
                "  border-radius: 8px; padding: 8px;"
                "  background-color: %1; border: none;"
                "}")
            .arg(barBg));

    updateButtonStyle();

    // Re-apply data item colors (muted color differs by theme)
    retranslateUi();
}
