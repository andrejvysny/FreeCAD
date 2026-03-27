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

#include <QComboBox>
#include <QFont>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QStyleHints>
#include <QTimer>
#include <QVBoxLayout>

#include "WizardBasicsPage.h"
#include "FirstStartWidget.h"
#include "NavigationCardsWidget.h"
#include "SelectionCard.h"

#include <App/Application.h>
#include <Base/Parameter.h>
#include <Base/UnitsApi.h>
#include <Gui/Application.h>
#include <Gui/Language/Translator.h>
#include <Gui/PreferencePackManager.h>

using namespace StartGui;

static bool isSystemInDarkMode()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    const auto scheme = QGuiApplication::styleHints()->colorScheme();
    return scheme == Qt::ColorScheme::Dark;
#elif QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    const QPalette defaultPalette;
    const auto text = defaultPalette.color(QPalette::WindowText);
    const auto window = defaultPalette.color(QPalette::Window);
    return text.lightness() > window.lightness();
#else
    return false;
#endif
}


WizardBasicsPage::WizardBasicsPage(QWidget* parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    // Language + Units row
    auto dropdownRow = new QHBoxLayout();
    dropdownRow->setSpacing(24);

    auto langCol = new QVBoxLayout();
    langCol->setSpacing(4);
    _languageLabel = new QLabel(this);
    QFont boldFont = _languageLabel->font();
    boldFont.setBold(true);
    _languageLabel->setFont(boldFont);
    langCol->addWidget(_languageLabel);
    createLanguageComboBox();
    _languageComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    langCol->addWidget(_languageComboBox);
    _languageLabel->setBuddy(_languageComboBox);
    dropdownRow->addLayout(langCol);

    auto unitsCol = new QVBoxLayout();
    unitsCol->setSpacing(4);
    _unitsLabel = new QLabel(this);
    _unitsLabel->setFont(boldFont);
    unitsCol->addWidget(_unitsLabel);
    createUnitSystemComboBox();
    _unitSystemComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    unitsCol->addWidget(_unitSystemComboBox);
    _unitsLabel->setBuddy(_unitSystemComboBox);
    dropdownRow->addLayout(unitsCol);

    layout->addLayout(dropdownRow);

    // Navigation cards
    _navigationCards = new NavigationCardsWidget(this);
    layout->addWidget(_navigationCards);

    // Theme section
    _themeLabel = new QLabel(this);
    _themeLabel->setFont(boldFont);
    layout->addWidget(_themeLabel);

    createThemeCards();
    auto themeRow = new QHBoxLayout();
    themeRow->setSpacing(12);
    _lightCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    _darkCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    themeRow->addWidget(_lightCard);
    themeRow->addWidget(_darkCard);
    layout->addLayout(themeRow);

    preselectTheme();
    retranslateUi();
}

void WizardBasicsPage::createLanguageComboBox()
{
    ParameterGrp::handle hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/General"
    );
    auto langToStr = Gui::Translator::instance()->activeLanguage();
    QByteArray language = hGrp->GetASCII("Language", langToStr.c_str()).c_str();

    _languageComboBox = new QComboBox(this);
    _languageComboBox->addItem(QStringLiteral("English"), QByteArray("English"));

    Gui::TStringMap list = Gui::Translator::instance()->supportedLocales();
    int index = 1;
    for (auto it = list.begin(); it != list.end(); ++it, ++index) {
        QByteArray lang = it->first.c_str();
        QString langname = QString::fromLatin1(lang.constData());

        if (it->second == "sr-CS") {
            it->second = "sr_Latn";
        }

        QLocale locale(QString::fromLatin1(it->second.c_str()));
        QString native = locale.nativeLanguageName();
        if (!native.isEmpty()) {
            if (native[0].isLetter()) {
                native[0] = native[0].toUpper();
            }
            langname = native;
        }

        _languageComboBox->addItem(langname, lang);
        if (language == lang) {
            _languageComboBox->setCurrentIndex(index);
        }
    }

    if (QAbstractItemModel* model = _languageComboBox->model()) {
        model->sort(0);
    }

    connect(
        _languageComboBox,
        qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        &WizardBasicsPage::onLanguageChanged
    );
}

void WizardBasicsPage::createUnitSystemComboBox()
{
    _unitSystemComboBox = new QComboBox(this);

    const ParameterGrp::handle hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Units"
    );
    auto userSchema = hGrp->GetInt("UserSchema", 0);

    int idx = 0;
    auto descriptions = Base::UnitsApi::getDescriptions();
    for (const auto& desc : descriptions) {
        _unitSystemComboBox->addItem(QString::fromStdString(desc), idx);
        ++idx;
    }
    _unitSystemComboBox->setCurrentIndex(userSchema);

    connect(
        _unitSystemComboBox,
        qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        &WizardBasicsPage::onUnitSystemChanged
    );
}

void WizardBasicsPage::createThemeCards()
{
    SelectionCard::Config lightCfg;
    lightCfg.title = tr("Light");
    lightCfg.icon = QIcon(QStringLiteral(":/thumbnails/theme-light-preview.svg"));
    lightCfg.iconSize = {140, 84};
    _lightCard = new SelectionCard(lightCfg, this);

    SelectionCard::Config darkCfg;
    darkCfg.title = tr("Dark");
    darkCfg.icon = QIcon(QStringLiteral(":/thumbnails/theme-dark-preview.svg"));
    darkCfg.iconSize = {140, 84};
    _darkCard = new SelectionCard(darkCfg, this);

    connect(_lightCard, &SelectionCard::clicked, this, [this]() { onThemeCardClicked(0); });
    connect(_darkCard, &SelectionCard::clicked, this, [this]() { onThemeCardClicked(1); });
}

void WizardBasicsPage::preselectTheme()
{
    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/MainWindow"
    );
    auto styleSheet = QString::fromStdString(hGrp->GetASCII("StyleSheet", "<N/A>"));

    if (styleSheet.contains(QLatin1String("Dark"), Qt::CaseInsensitive)) {
        _darkCard->setChecked(true);
    }
    else if (styleSheet.contains(QLatin1String("Light"), Qt::CaseInsensitive)) {
        _lightCard->setChecked(true);
    }
    else if (styleSheet == QLatin1String("<N/A>")) {
        // First launch — auto-detect
        if (isSystemInDarkMode()) {
            onThemeCardClicked(1);
            _darkCard->setChecked(true);
        }
        else {
            onThemeCardClicked(0);
            _lightCard->setChecked(true);
        }
    }
    else {
        _lightCard->setChecked(true);
    }
}

void WizardBasicsPage::onLanguageChanged(int index)
{
    if (index < 0) {
        return;
    }
    Gui::Translator::instance()->activateLanguage(
        _languageComboBox->itemData(index).toByteArray().data()
    );
    ParameterGrp::handle hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/General"
    );
    auto langToStr = Gui::Translator::instance()->activeLanguage();
    hGrp->SetASCII("Language", langToStr.c_str());
}

void WizardBasicsPage::onUnitSystemChanged(int index)
{
    if (index < 0) {
        return;
    }
    Base::UnitsApi::setSchema(index);
    ParameterGrp::handle hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Units"
    );
    hGrp->SetInt("UserSchema", index);
}

void WizardBasicsPage::onThemeCardClicked(int themeIndex)
{
    _lightCard->setChecked(themeIndex == 0);
    _darkCard->setChecked(themeIndex == 1);

    auto prefPackManager = Gui::Application::Instance->prefPackManager();
    if (themeIndex == 0) {
        prefPackManager->apply("FreeCAD Light");
    }
    else {
        prefPackManager->apply("FreeCAD Dark");
    }

    // Set default accent colors if not already set
    ParameterGrp::handle hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Themes"
    );
    const unsigned long nonExistentColor = -1434171135;
    const unsigned long defaultAccentColor = 1434171135;
    unsigned long accent = hGrp->GetUnsigned("ThemeAccentColor1", nonExistentColor);
    if (accent == nonExistentColor) {
        hGrp->SetUnsigned("ThemeAccentColor1", defaultAccentColor);
        hGrp->SetUnsigned("ThemeAccentColor2", defaultAccentColor);
        hGrp->SetUnsigned("ThemeAccentColor3", defaultAccentColor);
    }

    // Update wizard colors to match new theme
    auto* wizard = qobject_cast<FirstStartWidget*>(parentWidget());
    if (!wizard) {
        // May be nested deeper (inside QStackedWidget)
        QWidget* p = parentWidget();
        while (p && !wizard) {
            wizard = qobject_cast<FirstStartWidget*>(p);
            p = p->parentWidget();
        }
    }
    if (wizard) {
        // Small delay to let the theme fully apply
        QTimer::singleShot(100, wizard, &FirstStartWidget::applyThemeColors);
    }
}

void WizardBasicsPage::retranslateUi()
{
    _languageLabel->setText(tr("Language"));
    _unitsLabel->setText(tr("Units"));
    _themeLabel->setText(tr("Theme"));
    _lightCard->setTitle(tr("Light"));
    _darkCard->setTitle(tr("Dark"));
    _navigationCards->retranslateUi();

    // Rebuild unit system combo (names may change with language)
    _unitSystemComboBox->blockSignals(true);
    int currentUnit = _unitSystemComboBox->currentIndex();
    _unitSystemComboBox->clear();
    int idx = 0;
    auto descriptions = Base::UnitsApi::getDescriptions();
    for (const auto& desc : descriptions) {
        _unitSystemComboBox->addItem(QString::fromStdString(desc), idx);
        ++idx;
    }
    _unitSystemComboBox->setCurrentIndex(currentUnit);
    _unitSystemComboBox->blockSignals(false);
}
