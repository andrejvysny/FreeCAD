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
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QSignalBlocker>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>


#include <array>
#include <algorithm>
#include "GeneralSettingsWidget.h"
#include <gsl/pointers>
#include <App/Application.h>
#include <Base/Parameter.h>
#include <Base/UnitsApi.h>
#include <Gui/Language/Translator.h>
#include <Gui/Navigation/NavigationStyle.h>

using namespace StartGui;

namespace
{

constexpr auto orbitTranslationContext = "Gui::Dialog::DlgSettingsNavigation";

QString translateOrbitPreferenceText(const char* text)
{
    return QApplication::translate(orbitTranslationContext, text);
}

void setCurrentIndexByData(QComboBox* comboBox, const QVariant& value)
{
    if (!comboBox) {
        return;
    }

    const QSignalBlocker blocker(comboBox);
    const int index = comboBox->findData(value);
    if (index >= 0) {
        comboBox->setCurrentIndex(index);
    }
}

QWidget* createFieldWidget(
    QWidget* parent,
    QLabel* label,
    QLabel* helper,
    QComboBox* comboBox,
    const QString& widgetName
)
{
    auto fieldWidget = gsl::owner<QWidget*>(new QWidget(parent));
    fieldWidget->setObjectName(widgetName);

    auto layout = gsl::owner<QVBoxLayout*>(new QVBoxLayout(fieldWidget));
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);
    layout->addWidget(label);
    layout->addWidget(helper);
    layout->addWidget(comboBox);

    return fieldWidget;
}

}  // namespace

GeneralSettingsWidget::GeneralSettingsWidget(QWidget* parent)
    : QWidget(parent)
    , _languageLabel {nullptr}
    , _unitSystemLabel {nullptr}
    , _navigationStyleLabel {nullptr}
    , _orbitStyleLabel {nullptr}
    , _languageHelperLabel {nullptr}
    , _unitSystemHelperLabel {nullptr}
    , _navigationStyleHelperLabel {nullptr}
    , _orbitStyleHelperLabel {nullptr}
    , _languageComboBox {nullptr}
    , _unitSystemComboBox {nullptr}
    , _navigationStyleComboBox {nullptr}
    , _orbitStyleComboBox {nullptr}
{
    setObjectName(QLatin1String("GeneralSettingsWidget"));
    setupUi();
    qApp->installEventFilter(this);
}

void GeneralSettingsWidget::setupUi()
{
    auto mainLayout = gsl::owner<QVBoxLayout*>(new QVBoxLayout(this));
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(18);

    _languageLabel = gsl::owner<QLabel*>(new QLabel);
    _unitSystemLabel = gsl::owner<QLabel*>(new QLabel);
    _navigationStyleLabel = gsl::owner<QLabel*>(new QLabel);
    _orbitStyleLabel = gsl::owner<QLabel*>(new QLabel);
    _languageHelperLabel = gsl::owner<QLabel*>(new QLabel);
    _unitSystemHelperLabel = gsl::owner<QLabel*>(new QLabel);
    _navigationStyleHelperLabel = gsl::owner<QLabel*>(new QLabel);
    _orbitStyleHelperLabel = gsl::owner<QLabel*>(new QLabel);

    _languageLabel->setObjectName(QStringLiteral("firstStartLanguageLabel"));
    _languageLabel->setProperty("firstStartRole", QStringLiteral("field"));
    _unitSystemLabel->setObjectName(QStringLiteral("firstStartUnitsLabel"));
    _unitSystemLabel->setProperty("firstStartRole", QStringLiteral("field"));
    _navigationStyleLabel->setObjectName(QStringLiteral("firstStartNavigationLabel"));
    _navigationStyleLabel->setProperty("firstStartRole", QStringLiteral("field"));
    _orbitStyleLabel->setObjectName(QStringLiteral("firstStartOrbitStyleLabel"));
    _orbitStyleLabel->setProperty("firstStartRole", QStringLiteral("field"));

    _languageHelperLabel->setObjectName(QStringLiteral("firstStartLanguageHelperText"));
    _languageHelperLabel->setProperty("firstStartRole", QStringLiteral("helper"));
    _unitSystemHelperLabel->setObjectName(QStringLiteral("firstStartUnitsHelperText"));
    _unitSystemHelperLabel->setProperty("firstStartRole", QStringLiteral("helper"));
    _navigationStyleHelperLabel->setObjectName(QStringLiteral("firstStartNavigationHelperText"));
    _navigationStyleHelperLabel->setProperty("firstStartRole", QStringLiteral("helper"));
    _orbitStyleHelperLabel->setObjectName(QStringLiteral("firstStartOrbitStyleHelperText"));
    _orbitStyleHelperLabel->setProperty("firstStartRole", QStringLiteral("helper"));

    createLanguageComboBox();
    createUnitSystemComboBox();
    createNavigationStyleComboBox();
    createOrbitStyleComboBox();

    auto basicsRow = gsl::owner<QHBoxLayout*>(new QHBoxLayout);
    basicsRow->setContentsMargins(0, 0, 0, 0);
    basicsRow->setSpacing(16);
    basicsRow->addWidget(
        createFieldWidget(
            this,
            _languageLabel,
            _languageHelperLabel,
            _languageComboBox,
            QStringLiteral("firstStartLanguageField")
        )
    );
    basicsRow->addWidget(
        createFieldWidget(
            this,
            _unitSystemLabel,
            _unitSystemHelperLabel,
            _unitSystemComboBox,
            QStringLiteral("firstStartUnitsField")
        )
    );
    mainLayout->addLayout(basicsRow);
    auto navigationRow = gsl::owner<QHBoxLayout*>(new QHBoxLayout);
    navigationRow->setContentsMargins(0, 0, 0, 0);
    navigationRow->setSpacing(16);
    navigationRow->addWidget(
        createFieldWidget(
            this,
            _navigationStyleLabel,
            _navigationStyleHelperLabel,
            _navigationStyleComboBox,
            QStringLiteral("firstStartNavigationField")
        )
    );
    navigationRow->addWidget(
        createFieldWidget(
            this,
            _orbitStyleLabel,
            _orbitStyleHelperLabel,
            _orbitStyleComboBox,
            QStringLiteral("firstStartOrbitStyleField")
        )
    );
    mainLayout->addLayout(navigationRow);

    retranslateUi();
}

gsl::owner<QComboBox*> GeneralSettingsWidget::createLanguageComboBox()
{
    ParameterGrp::handle hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/General"
    );
    auto langToStr = Gui::Translator::instance()->activeLanguage();
    QByteArray language = hGrp->GetASCII("Language", langToStr.c_str()).c_str();
    auto comboBox = gsl::owner<QComboBox*>(new QComboBox);
    comboBox->setObjectName(QStringLiteral("firstStartLanguageComboBox"));
    comboBox->addItem(QStringLiteral("English"), QByteArray("English"));
    Gui::TStringMap list = Gui::Translator::instance()->supportedLocales();
    for (auto it = list.begin(); it != list.end(); ++it) {
        QByteArray lang = it->first.c_str();
        QString langname = QString::fromLatin1(lang.constData());

        if (it->second == "sr-CS") {
            // Qt does not treat sr-CS (Serbian, Latin) as a Latin-script variant by default: this
            // forces it to do so.
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

        comboBox->addItem(langname, lang);
    }
    if (QAbstractItemModel* model = comboBox->model()) {
        model->sort(0);
    }
    _languageComboBox = comboBox;
    connect(
        _languageComboBox,
        qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        &GeneralSettingsWidget::onLanguageChanged
    );
    setCurrentIndexByData(_languageComboBox, language);
    return comboBox;
}

gsl::owner<QComboBox*> GeneralSettingsWidget::createUnitSystemComboBox()
{
    // Contents are created in retranslateUi()
    auto comboBox = gsl::owner<QComboBox*>(new QComboBox);
    comboBox->setObjectName(QStringLiteral("firstStartUnitSystemComboBox"));
    _unitSystemComboBox = comboBox;
    connect(
        _unitSystemComboBox,
        qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        &GeneralSettingsWidget::onUnitSystemChanged
    );
    return comboBox;
}

gsl::owner<QComboBox*> GeneralSettingsWidget::createNavigationStyleComboBox()
{
    // Contents are created in retranslateUi()
    auto comboBox = gsl::owner<QComboBox*>(new QComboBox);
    comboBox->setObjectName(QStringLiteral("firstStartNavigationStyleComboBox"));
    _navigationStyleComboBox = comboBox;
    connect(
        _navigationStyleComboBox,
        qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        &GeneralSettingsWidget::onNavigationStyleChanged
    );
    return comboBox;
}

gsl::owner<QComboBox*> GeneralSettingsWidget::createOrbitStyleComboBox()
{
    auto comboBox = gsl::owner<QComboBox*>(new QComboBox);
    comboBox->setObjectName(QStringLiteral("firstStartOrbitStyleComboBox"));
    _orbitStyleComboBox = comboBox;
    connect(
        _orbitStyleComboBox,
        qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        &GeneralSettingsWidget::onOrbitStyleChanged
    );
    return comboBox;
}

void GeneralSettingsWidget::populateUnitSystemComboBox()
{
    const QSignalBlocker blocker(_unitSystemComboBox);
    _unitSystemComboBox->clear();

    auto addItem = [&, index {0}](const std::string& item) mutable {
        _unitSystemComboBox->addItem(QString::fromStdString(item), index++);
    };
    auto descriptions = Base::UnitsApi::getDescriptions();
    std::for_each(descriptions.begin(), descriptions.end(), addItem);
}

void GeneralSettingsWidget::populateNavigationStyleComboBox()
{
    const QSignalBlocker blocker(_navigationStyleComboBox);
    _navigationStyleComboBox->clear();

    std::map<Base::Type, std::string> styles = Gui::UserNavigationStyle::getUserFriendlyNames();
    for (const auto& style : styles) {
        QByteArray data(style.first.getName());
        QString name = QApplication::translate(style.first.getName(), style.second.c_str());
        _navigationStyleComboBox->addItem(name, data);
    }
}

void GeneralSettingsWidget::populateOrbitStyleComboBox()
{
    const QSignalBlocker blocker(_orbitStyleComboBox);
    _orbitStyleComboBox->clear();

    const std::array<const char*, 5> orbitStyles {
        "Turntable",
        "Trackball",
        "Free Turntable",
        "Trackball Classic",
        "Rounded Arcball"
    };
    for (std::size_t index = 0; index < orbitStyles.size(); ++index) {
        _orbitStyleComboBox->addItem(
            translateOrbitPreferenceText(orbitStyles[index]),
            static_cast<int>(index)
        );
    }
}

void GeneralSettingsWidget::onLanguageChanged(int index)
{
    if (index < 0) {
        return;  // happens when clearing the combo box in retranslateUi()
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

void GeneralSettingsWidget::onUnitSystemChanged(int index)
{
    if (index < 0) {
        return;  // happens when clearing the combo box in retranslateUi()
    }
    Base::UnitsApi::setSchema(index);
    ParameterGrp::handle hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Units"
    );
    hGrp->SetInt("UserSchema", index);
}

void GeneralSettingsWidget::onNavigationStyleChanged(int index)
{
    if (index < 0) {
        return;  // happens when clearing the combo box in retranslateUi()
    }
    auto navStyleName = _navigationStyleComboBox->itemData(index).toByteArray();
    ParameterGrp::handle hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/View"
    );
    hGrp->SetASCII("NavigationStyle", navStyleName.constData());
}

void GeneralSettingsWidget::onOrbitStyleChanged(int index)
{
    if (index < 0) {
        return;
    }
    ParameterGrp::handle hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/View"
    );
    hGrp->SetInt("OrbitStyle", _orbitStyleComboBox->itemData(index).toInt());
}

bool GeneralSettingsWidget::eventFilter(QObject* object, QEvent* event)
{
    if (object == this && event->type() == QEvent::LanguageChange) {
        this->retranslateUi();
    }
    return QWidget::eventFilter(object, event);
}

void GeneralSettingsWidget::refreshFromPreferences()
{
    ParameterGrp::handle hGrpGeneral = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/General"
    );
    auto activeLanguage = Gui::Translator::instance()->activeLanguage();
    QByteArray language
        = hGrpGeneral->GetASCII("Language", activeLanguage.c_str()).c_str();
    setCurrentIndexByData(_languageComboBox, language);

    const ParameterGrp::handle hGrpUnits = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Units"
    );
    auto userSchema = static_cast<int>(hGrpUnits->GetInt("UserSchema", 0));
    {
        const QSignalBlocker blocker(_unitSystemComboBox);
        if (userSchema >= 0 && userSchema < _unitSystemComboBox->count()) {
            _unitSystemComboBox->setCurrentIndex(userSchema);
        }
    }

    ParameterGrp::handle hGrpNav = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/View"
    );
    QByteArray navStyleName = hGrpNav->GetASCII(
        "NavigationStyle",
        Gui::CADNavigationStyle::getClassTypeId().getName()
    ).c_str();
    setCurrentIndexByData(_navigationStyleComboBox, navStyleName);

    int orbitStyle = static_cast<int>(
        hGrpNav->GetInt("OrbitStyle", int(Gui::NavigationStyle::RoundedArcball))
    );
    orbitStyle = std::clamp(orbitStyle, 0, _orbitStyleComboBox->count() - 1);
    setCurrentIndexByData(_orbitStyleComboBox, orbitStyle);
}

void GeneralSettingsWidget::retranslateUi()
{
    _languageLabel->setText(tr("Language"));
    _languageHelperLabel->setText(tr("Used for UI text"));

    _unitSystemLabel->setText(tr("Units"));
    _unitSystemHelperLabel->setText(tr("Affects dimensions and measurements"));
    populateUnitSystemComboBox();

    _navigationStyleLabel->setText(tr("Navigation Style"));
    _navigationStyleHelperLabel->setText(tr("Controls mouse behavior"));
    populateNavigationStyleComboBox();

    auto orbitStyleTooltip = translateOrbitPreferenceText(
        "Rotation orbit style.\n"
        "Rounded Arcball: moving the mouse in the corners of the screen will only roll the part.\n"
        "Trackball: moving the mouse horizontally will rotate the part around the Y-axis.\n"
        "Trackball Classic: moving the mouse will rotate the part allowing precession.\n"
        "Turntable: the part will be rotated around the Z-axis (with constrained axes).\n"
        "Free Turntable: the part will be rotated around the Z-axis.\n"
        "         "
    );
    _orbitStyleLabel->setText(translateOrbitPreferenceText("Orbit style"));
    _orbitStyleHelperLabel->setText(tr("Controls rotation behavior"));
    _orbitStyleComboBox->setToolTip(orbitStyleTooltip);
    populateOrbitStyleComboBox();

    refreshFromPreferences();
}
