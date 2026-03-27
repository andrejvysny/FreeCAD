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
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPalette>
#include <QSizePolicy>
#include <QString>
#include <QStyleHints>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

#include "ThemeSelectorWidget.h"
#include <gsl/pointers>
#include <App/Application.h>
#include <Gui/Application.h>
#include <Gui/Command.h>
#include <Gui/PreferencePackManager.h>

#include <FCConfig.h>

#ifdef FC_OS_MACOSX
# include <CoreFoundation/CoreFoundation.h>
#endif

using namespace StartGui;


namespace
{

void persistThemeSelection(Theme theme)
{
    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/MainWindow"
    );

    switch (theme) {
        case Theme::Classic:
            hGrp->SetASCII("Theme", "Classic");
            break;
        case Theme::Light:
            hGrp->SetASCII("Theme", "FreeCAD Light");
            break;
        case Theme::Dark:
            hGrp->SetASCII("Theme", "FreeCAD Dark");
            break;
    }
}

bool hasStoredThemePreference()
{
    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/MainWindow"
    );
    return !QString::fromStdString(hGrp->GetASCII("Theme", nullptr)).isEmpty()
        || !QString::fromStdString(hGrp->GetASCII("StyleSheet", nullptr)).isEmpty();
}

}  // namespace


static bool isSystemInDarkMode()
{
    // Auto-detect system setting and default to light mode
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    // https://www.qt.io/blog/dark-mode-on-windows-11-with-qt-6.5
    const auto scheme = QGuiApplication::styleHints()->colorScheme();
    return scheme == Qt::ColorScheme::Dark;
#elif QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    // https://www.qt.io/blog/dark-mode-on-windows-11-with-qt-6.5
    const QPalette defaultPalette;
    const auto text = defaultPalette.color(QPalette::WindowText);
    const auto window = defaultPalette.color(QPalette::Window);
    return text.lightness() > window.lightness();
#else
# ifdef FC_OS_MACOSX
    auto key = CFSTR("AppleInterfaceStyle");
    if (auto value = CFPreferencesCopyAppValue(key, kCFPreferencesAnyApplication)) {
        // If the value is "Dark", Dark Mode is enabled
        if (CFGetTypeID(value) == CFStringGetTypeID()) {
            if (CFStringCompare((CFStringRef)value, CFSTR("Dark"), kCFCompareCaseInsensitive)
                == kCFCompareEqualTo) {
                CFRelease(value);
                return true;
            }
        }
        CFRelease(value);
    }
# endif  // FC_OS_MACOSX
#endif   // QT_VERSION >= 6.4+
    return false;
}


static bool shouldHideClassicTheme()
{
    // Classic on macOS and windows 11 with qt6(.4+?) doesn't work when system
    // is in dark mode and to make matter worse, on macOS there's a setting that
    // changes mode depending on time of day.
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0) || defined(FC_OS_MACOSX) || defined(FC_OS_WIN32)
    return true;
#else
    return false;
#endif
}


ThemeSelectorWidget::ThemeSelectorWidget(QWidget* parent)
    : QWidget(parent)
    , _titleLabel {nullptr}
    , _helperLabel {nullptr}
    , _addonManagerLabel {nullptr}
    , _buttons {nullptr, nullptr, nullptr}
{
    setObjectName(QLatin1String("ThemeSelectorWidget"));
    if (shouldHideClassicTheme()) {
        preselectThemeFromSystemSettings();
    }
    setupUi();
    qApp->installEventFilter(this);
}


void ThemeSelectorWidget::setupButtons(QLayout* layout)
{
    if (!layout) {
        return;
    }
    std::map<Theme, QString> themeMap {
        {Theme::Classic, tr("FreeCAD Classic")},
        {Theme::Dark, tr("FreeCAD Dark")},
        {Theme::Light, tr("FreeCAD Light")}
    };
    std::map<Theme, QIcon> iconMap {
        {Theme::Classic, QIcon(QLatin1String(":/thumbnails/Theme_thumbnail_classic.png"))},
        {Theme::Light, QIcon(QLatin1String(":/thumbnails/Theme_thumbnail_light.png"))},
        {Theme::Dark, QIcon(QLatin1String(":/thumbnails/Theme_thumbnail_dark.png"))}
    };
    for (const auto& theme : themeMap) {
        auto button = gsl::owner<QToolButton*>(new QToolButton());

        button->setCheckable(true);
        button->setAutoExclusive(true);
        button->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
        button->setText(theme.second);
        button->setIcon(iconMap[theme.first]);
        button->setIconSize(iconMap[theme.first].actualSize(QSize(128, 128)));
        button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        button->setMinimumWidth(156);
        button->setMaximumWidth(156);
        switch (theme.first) {
            case Theme::Classic:
                button->setObjectName(QStringLiteral("firstStartThemeClassicButton"));
                break;
            case Theme::Light:
                button->setObjectName(QStringLiteral("firstStartThemeLightButton"));
                break;
            case Theme::Dark:
                button->setObjectName(QStringLiteral("firstStartThemeDarkButton"));
                break;
        }
        connect(button, &QToolButton::clicked, this, [this, theme] { themeChanged(theme.first); });
        layout->addWidget(button);
        _buttons[static_cast<int>(theme.first)] = button;
    }
}

void ThemeSelectorWidget::setupUi()
{
    auto* outerLayout = gsl::owner<QVBoxLayout*>(new QVBoxLayout(this));
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(6);

    _titleLabel = gsl::owner<QLabel*>(new QLabel);
    _titleLabel->setObjectName(QStringLiteral("firstStartAppearanceLabel"));
    _titleLabel->setProperty("firstStartRole", QStringLiteral("subsection"));

    _helperLabel = gsl::owner<QLabel*>(new QLabel);
    _helperLabel->setObjectName(QStringLiteral("firstStartAppearanceHelperText"));
    _helperLabel->setProperty("firstStartRole", QStringLiteral("helper"));
    _helperLabel->setWordWrap(true);
    _helperLabel->setMaximumWidth(460);

    _addonManagerLabel = gsl::owner<QLabel*>(new QLabel);
    _addonManagerLabel->setObjectName(QStringLiteral("firstStartAppearanceAddonText"));
    _addonManagerLabel->setProperty("firstStartRole", QStringLiteral("helper"));
    _addonManagerLabel->setWordWrap(true);
    _addonManagerLabel->setMaximumWidth(520);

    auto* themeCardsWidget = gsl::owner<QWidget*>(new QWidget(this));
    themeCardsWidget->setObjectName(QStringLiteral("firstStartThemeCardsWidget"));
    themeCardsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    auto* buttonLayout = gsl::owner<QHBoxLayout*>(new QHBoxLayout(themeCardsWidget));
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(10);

    outerLayout->addWidget(_titleLabel);
    outerLayout->addWidget(_helperLabel);
    outerLayout->addWidget(themeCardsWidget);
    outerLayout->addWidget(_addonManagerLabel);
    buttonLayout->addStretch();
    setupButtons(buttonLayout);
    buttonLayout->addStretch();
    retranslateUi();
    refreshFromPreferences();
    connect(_addonManagerLabel, &QLabel::linkActivated, this, &ThemeSelectorWidget::onLinkActivated);
}

void ThemeSelectorWidget::onLinkActivated(const QString& link)
{
    auto const addonManagerLink = QStringLiteral("freecad:Std_AddonMgr");

    if (link != addonManagerLink) {
        return;
    }

    // Set the user preferences to include only preference packs.
    // This is a quick and dirty way to open Addon Manager with only themes.
    auto pref = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Addons"
    );
    pref->SetInt("PackageTypeSelection", 3);  // 3 stands for Preference Packs
    pref->SetInt("StatusSelection", 0);       // 0 stands for any installation status

    if (!hasAddonManagerCommand()) {
        return;
    }

    Gui::Application::Instance->commandManager().runCommandByName("Std_AddonMgr");
}

void ThemeSelectorWidget::preselectThemeFromSystemSettings()
{
    if (!hasStoredThemePreference()) {
        auto defaultTheme = isSystemInDarkMode() ? Theme::Dark : Theme::Light;
        if (Gui::Application::Instance && Gui::Application::Instance->prefPackManager()) {
            themeChanged(defaultTheme);
        }
        else {
            persistThemeSelection(defaultTheme);
        }
    }
}

void ThemeSelectorWidget::themeChanged(Theme newTheme)
{
    if (Gui::Application::Instance && Gui::Application::Instance->prefPackManager()) {
        // Run the appropriate preference pack.
        auto prefPackManager = Gui::Application::Instance->prefPackManager();
        switch (newTheme) {
            case Theme::Classic:
                prefPackManager->apply("FreeCAD Classic");
                break;
            case Theme::Dark:
                prefPackManager->apply("FreeCAD Dark");
                break;
            case Theme::Light:
                prefPackManager->apply("FreeCAD Light");
                break;
        }
    }
    else {
        persistThemeSelection(newTheme);
    }

    ParameterGrp::handle hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Themes"
    );
    const unsigned long nonExistentColor = -1434171135;
    const unsigned long defaultAccentColor = 1434171135;
    unsigned long longAccentColor1 = hGrp->GetUnsigned("ThemeAccentColor1", nonExistentColor);
    if (longAccentColor1 == nonExistentColor) {
        hGrp->SetUnsigned("ThemeAccentColor1", defaultAccentColor);
        hGrp->SetUnsigned("ThemeAccentColor2", defaultAccentColor);
        hGrp->SetUnsigned("ThemeAccentColor3", defaultAccentColor);
    }

    refreshFromPreferences();
    Q_EMIT themeApplied();
}

bool ThemeSelectorWidget::eventFilter(QObject* object, QEvent* event)
{
    if (object == this && event->type() == QEvent::LanguageChange) {
        this->retranslateUi();
    }
    return QWidget::eventFilter(object, event);
}

Theme ThemeSelectorWidget::currentThemeFromPreferences() const
{
    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/MainWindow"
    );
    auto themeName = QString::fromStdString(hGrp->GetASCII("Theme", nullptr));
    if (themeName.contains(QLatin1String("Dark"), Qt::CaseInsensitive)) {
        return Theme::Dark;
    }
    if (themeName.contains(QLatin1String("Light"), Qt::CaseInsensitive)) {
        return Theme::Light;
    }

    auto styleSheetName = QString::fromStdString(hGrp->GetASCII("StyleSheet", nullptr));
    if (styleSheetName.contains(QLatin1String("Dark"), Qt::CaseInsensitive)) {
        return Theme::Dark;
    }
    if (styleSheetName.contains(QLatin1String("Light"), Qt::CaseInsensitive)) {
        return Theme::Light;
    }

    return Theme::Classic;
}

bool ThemeSelectorWidget::hasAddonManagerCommand() const
{
    return Gui::Application::Instance
        && Gui::Application::Instance->commandManager().getCommandByName("Std_AddonMgr");
}

void ThemeSelectorWidget::refreshFromPreferences()
{
    auto theme = currentThemeFromPreferences();
    if (auto* button = _buttons[static_cast<int>(theme)]) {
        button->setChecked(true);
    }
}

void ThemeSelectorWidget::retranslateUi()
{
    _titleLabel->setText(tr("Appearance"));
    _helperLabel->setText(tr("Choose how FreeCAD looks"));
    if (hasAddonManagerCommand()) {
        _addonManagerLabel->setText(
            tr("Looking for more themes? You can obtain them using "
               "<a href=\"freecad:Std_AddonMgr\">Addon Manager</a>.")
        );
        _addonManagerLabel->show();
    }
    else {
        _addonManagerLabel->hide();
    }
    _buttons[static_cast<int>(Theme::Dark)]->setText(tr("FreeCAD Dark", "Visual theme name"));
    _buttons[static_cast<int>(Theme::Light)]->setText(tr("FreeCAD Light", "Visual theme name"));
    _buttons[static_cast<int>(Theme::Classic)]->setText(tr("FreeCAD Classic", "Visual theme name"));
}
