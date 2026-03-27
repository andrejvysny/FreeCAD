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

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPalette>
#include <QPixmap>
#include <QPushButton>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QWidget>


#include "FirstStartWidget.h"
#include "GeneralSettingsWidget.h"
#include "ThemeSelectorWidget.h"

#include <App/Application.h>
#include <Gui/Application.h>
#include <Gui/Command.h>
#include <gsl/pointers>

using namespace StartGui;

namespace
{

bool shouldUseLightWordmark(QLabel* wordmarkLabel)
{
    if (!wordmarkLabel) {
        return false;
    }

    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/MainWindow"
    );
    auto theme = QString::fromStdString(hGrp->GetASCII("Theme", "Classic"));

    if (theme.contains(QLatin1String("Dark"), Qt::CaseInsensitive)) {
        return true;
    }
    if (theme.contains(QLatin1String("Light"), Qt::CaseInsensitive)) {
        return false;
    }

    QColor bg = wordmarkLabel->palette().color(QPalette::Window);
    double luminance = 0.299 * bg.redF() + 0.587 * bg.greenF() + 0.114 * bg.blueF();
    return luminance < 0.5;
}

}  // namespace

FirstStartWidget::FirstStartWidget(QWidget* parent)
    : QGroupBox(parent)
    , _themeSelectorWidget {nullptr}
    , _generalSettingsWidget {nullptr}
    , _wordmarkLabel {nullptr}
    , _descriptionLabel {nullptr}
    , _buttonRowWidget {nullptr}
    , _advancedSettingsButton {nullptr}
    , _doneButton {nullptr}
{
    setObjectName(QLatin1String("FirstStartWidget"));
    setupUi();
    qApp->installEventFilter(this);
}

void FirstStartWidget::setupUi()
{
    static constexpr int modalMaximumWidth = 760;
    static constexpr int introMaximumWidth = 560;

    setTitle(QString());
    setMaximumWidth(modalMaximumWidth);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    auto outerLayout = gsl::owner<QVBoxLayout*>(new QVBoxLayout(this));
    outerLayout->setSizeConstraint(QLayout::SizeConstraint::SetMinAndMaxSize);
    outerLayout->setContentsMargins(20, 18, 20, 18);
    outerLayout->setSpacing(10);
    outerLayout->setAlignment(Qt::AlignTop);

    _wordmarkLabel = gsl::owner<QLabel*>(new QLabel(this));
    _wordmarkLabel->setObjectName(QStringLiteral("firstStartWordmark"));
    _wordmarkLabel->setFixedHeight(60);
    _wordmarkLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    outerLayout->addWidget(_wordmarkLabel);

    _descriptionLabel = gsl::owner<QLabel*>(new QLabel(this));
    _descriptionLabel->setObjectName(QStringLiteral("firstStartIntro"));
    _descriptionLabel->setProperty("firstStartRole", QStringLiteral("intro"));
    _descriptionLabel->setWordWrap(true);
    _descriptionLabel->setMaximumWidth(introMaximumWidth);
    outerLayout->addWidget(_descriptionLabel);

    _generalSettingsWidget = gsl::owner<GeneralSettingsWidget*>(new GeneralSettingsWidget(this));
    _generalSettingsWidget->setMaximumWidth(modalMaximumWidth - 40);
    outerLayout->addWidget(_generalSettingsWidget);

    outerLayout->addSpacing(4);

    _themeSelectorWidget = gsl::owner<ThemeSelectorWidget*>(new ThemeSelectorWidget(this));
    outerLayout->addWidget(_themeSelectorWidget);
    connect(_themeSelectorWidget, &ThemeSelectorWidget::themeApplied, this, &FirstStartWidget::updateWordmark);

    _buttonRowWidget = gsl::owner<QWidget*>(new QWidget(this));
    _buttonRowWidget->setObjectName(QStringLiteral("firstStartActionRow"));
    _doneButton = gsl::owner<QPushButton*>(new QPushButton);
    _doneButton->setObjectName(QStringLiteral("firstStartPrimaryButton"));
    _doneButton->setDefault(true);
    connect(_doneButton, &QPushButton::clicked, this, &FirstStartWidget::dismissed);

    _advancedSettingsButton = gsl::owner<QPushButton*>(new QPushButton);
    _advancedSettingsButton->setObjectName(QStringLiteral("firstStartSecondaryButton"));
    connect(
        _advancedSettingsButton,
        &QPushButton::clicked,
        this,
        &FirstStartWidget::onAdvancedSettingsClicked
    );

    auto buttonBar = gsl::owner<QHBoxLayout*>(new QHBoxLayout(_buttonRowWidget));
    buttonBar->setContentsMargins(0, 2, 0, 0);
    buttonBar->setSpacing(8);
    buttonBar->addStretch();
    buttonBar->addWidget(_advancedSettingsButton);
    buttonBar->addWidget(_doneButton);
    outerLayout->addWidget(_buttonRowWidget);

    retranslateUi();
    refreshFromPreferences();
}

bool FirstStartWidget::eventFilter(QObject* object, QEvent* event)
{
    if (object == this && event->type() == QEvent::LanguageChange) {
        this->retranslateUi();
    }
    if (object == this
        && (event->type() == QEvent::PaletteChange || event->type() == QEvent::StyleChange)) {
        updateWordmark();
    }
    return QWidget::eventFilter(object, event);
}

bool FirstStartWidget::openAdvancedSettings()
{
    if (!Gui::Application::Instance) {
        return false;
    }

    auto& commandManager = Gui::Application::Instance->commandManager();
    if (!commandManager.getCommandByName("Std_DlgPreferences")) {
        return false;
    }

    commandManager.runCommandByName("Std_DlgPreferences");
    return true;
}

void FirstStartWidget::updateWordmark()
{
    QString wordmarkPath = shouldUseLightWordmark(_wordmarkLabel)
        ? QStringLiteral(":/branding/FreeCAD-wordmark-light.svg")
        : QStringLiteral(":/branding/FreeCAD-wordmark.svg");
    QPixmap wordmark(wordmarkPath);
    if (!wordmark.isNull()) {
        _wordmarkLabel->setPixmap(wordmark.scaledToHeight(52, Qt::SmoothTransformation));
    }
}

void FirstStartWidget::refreshFromPreferences()
{
    _generalSettingsWidget->refreshFromPreferences();
    _themeSelectorWidget->refreshFromPreferences();
    updateWordmark();
}

void FirstStartWidget::onAdvancedSettingsClicked()
{
    if (openAdvancedSettings()) {
        refreshFromPreferences();
    }
}

void FirstStartWidget::retranslateUi()
{
    _advancedSettingsButton->setText(tr("Advanced Settings"));
    _doneButton->setText(tr("Start Using FreeCAD"));
    _descriptionLabel->setText(
        tr("Set your basic configuration options below. These options can be changed later in the "
           "preferences.")
    );
}
