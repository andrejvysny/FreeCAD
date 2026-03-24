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

#include <QApplication>
#include <QComboBox>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QSizePolicy>
#include <QVBoxLayout>

#include "NavigationCardsWidget.h"
#include "SelectionCard.h"

#include <App/Application.h>
#include <Base/Parameter.h>
#include <Base/Type.h>
#include <Gui/Navigation/NavigationStyle.h>

using namespace StartGui;

// Navigation style class names for the 3 featured cards
static const char* featuredStyles[] = {
    "Gui::CADNavigationStyle",
    "Gui::BlenderNavigationStyle",
    "Gui::TouchpadNavigationStyle",
};
static constexpr int numFeatured = 3;

// Short display names for featured cards
static const char* featuredNames[] = {
    QT_TR_NOOP("CAD"),
    QT_TR_NOOP("Blender"),
    QT_TR_NOOP("Touchpad"),
};

// SVG icons for featured cards
static const char* featuredIcons[] = {
    ":/icons/nav-cad.svg",
    ":/icons/nav-blender.svg",
    ":/icons/nav-touchpad.svg",
};


NavigationCardsWidget::NavigationCardsWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("NavigationCardsWidget"));

    auto outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(8);

    // Title
    _titleLabel = new QLabel(this);
    QFont titleFont = _titleLabel->font();
    titleFont.setBold(true);
    _titleLabel->setFont(titleFont);
    outerLayout->addWidget(_titleLabel);

    // Subtitle
    _subtitleLabel = new QLabel(this);
    _subtitleLabel->setObjectName(QStringLiteral("wizardSubtitle"));
    outerLayout->addWidget(_subtitleLabel);

    // Cards row
    auto cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(12);

    for (int i = 0; i < numFeatured; ++i) {
        QIcon icon = QIcon(QString::fromLatin1(featuredIcons[i]));

        SelectionCard::Config cfg;
        cfg.title = tr(featuredNames[i]);
        cfg.icon = icon;
        cfg.iconSize = {48, 48};
        auto card = new SelectionCard(cfg, this);

        // Add orbit/pan/zoom mapping text as content
        auto mappingWidget = createMappingWidget(featuredStyles[i]);
        if (mappingWidget) {
            card->setContentWidget(mappingWidget);
        }

        connect(card, &SelectionCard::clicked, this, [this, i]() { onCardClicked(i); });

        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        _cards.push_back(card);
        _cardStyleNames.push_back(QByteArray(featuredStyles[i]));
        cardsLayout->addWidget(card);
    }

    outerLayout->addLayout(cardsLayout);

    // "Other styles..." dropdown
    auto otherRow = new QHBoxLayout();
    _otherStylesCombo = new QComboBox(this);
    _otherStylesCombo->setObjectName(QStringLiteral("wizardOtherNavCombo"));

    // Populate with non-featured styles
    std::map<Base::Type, std::string> styles = Gui::UserNavigationStyle::getUserFriendlyNames();
    for (const auto& style : styles) {
        QByteArray name(style.first.getName());
        bool isFeatured = false;
        for (int i = 0; i < numFeatured; ++i) {
            if (name == featuredStyles[i]) {
                isFeatured = true;
                break;
            }
        }
        if (!isFeatured) {
            QString displayName = QApplication::translate(style.first.getName(), style.second.c_str());
            _otherStylesCombo->addItem(displayName, name);
        }
    }

    _otherStylesCombo->setPlaceholderText(tr("Other styles..."));
    _otherStylesCombo->setCurrentIndex(-1);

    connect(
        _otherStylesCombo,
        qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        &NavigationCardsWidget::onDropdownChanged
    );

    otherRow->addWidget(_otherStylesCombo);
    otherRow->addStretch();
    outerLayout->addLayout(otherRow);

    retranslateUi();
    preselectCurrentStyle();
}

QWidget* NavigationCardsWidget::createMappingWidget(const char* className)
{
    Base::Type type = Base::Type::fromName(className);
    if (type.isBad()) {
        return nullptr;
    }

    auto* style = static_cast<Gui::UserNavigationStyle*>(type.createInstance());
    if (!style) {
        return nullptr;
    }

    auto widget = new QWidget();
    auto layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 4, 0, 0);
    layout->setSpacing(2);

    struct MappingRow {
        const char* label;
        Gui::NavigationStyle::ViewerMode mode;
    };
    MappingRow rows[] = {
        {QT_TR_NOOP("Orbit"), Gui::NavigationStyle::DRAGGING},
        {QT_TR_NOOP("Pan"), Gui::NavigationStyle::PANNING},
        {QT_TR_NOOP("Zoom"), Gui::NavigationStyle::ZOOMING},
    };

    for (const auto& row : rows) {
        auto rowLayout = new QHBoxLayout();
        rowLayout->setSpacing(8);
        rowLayout->setContentsMargins(0, 0, 0, 0);

        auto label = new QLabel(tr(row.label));
        QFont labelFont = label->font();
        labelFont.setPointSize(9);
        labelFont.setBold(true);
        label->setFont(labelFont);
        label->setFixedWidth(36);

        auto value = new QLabel(QApplication::translate(className, style->mouseButtons(row.mode)));
        QFont valueFont = value->font();
        valueFont.setPointSize(9);
        value->setFont(valueFont);
        value->setWordWrap(true);

        rowLayout->addWidget(label);
        rowLayout->addWidget(value, 1);
        layout->addLayout(rowLayout);
    }

    delete style;
    return widget;
}

void NavigationCardsWidget::onCardClicked(int cardIndex)
{
    // Deselect all other cards
    for (int i = 0; i < static_cast<int>(_cards.size()); ++i) {
        _cards[static_cast<size_t>(i)]->setChecked(i == cardIndex);
    }
    // Reset dropdown (block signal to avoid re-entrant call)
    _otherStylesCombo->blockSignals(true);
    _otherStylesCombo->setCurrentIndex(-1);
    _otherStylesCombo->blockSignals(false);

    applyNavigationStyle(_cardStyleNames[static_cast<size_t>(cardIndex)]);
}

void NavigationCardsWidget::onDropdownChanged(int comboIndex)
{
    if (comboIndex < 0) {
        return;
    }
    // Deselect all cards
    for (auto* card : _cards) {
        card->setChecked(false);
    }
    auto styleName = _otherStylesCombo->itemData(comboIndex).toByteArray();
    applyNavigationStyle(styleName);
}

void NavigationCardsWidget::applyNavigationStyle(const QByteArray& styleName)
{
    ParameterGrp::handle hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/View"
    );
    hGrp->SetASCII("NavigationStyle", styleName.constData());
}

void NavigationCardsWidget::preselectCurrentStyle()
{
    ParameterGrp::handle hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/View"
    );
    auto current = QByteArray::fromStdString(
        hGrp->GetASCII("NavigationStyle", Gui::CADNavigationStyle::getClassTypeId().getName())
    );

    bool found = false;
    for (int i = 0; i < static_cast<int>(_cards.size()); ++i) {
        if (_cardStyleNames[static_cast<size_t>(i)] == current) {
            _cards[static_cast<size_t>(i)]->setChecked(true);
            found = true;
            break;
        }
    }

    if (!found) {
        // Select in dropdown
        for (int i = 0; i < _otherStylesCombo->count(); ++i) {
            if (_otherStylesCombo->itemData(i).toByteArray() == current) {
                _otherStylesCombo->setCurrentIndex(i);
                break;
            }
        }
    }
}

void NavigationCardsWidget::retranslateUi()
{
    _titleLabel->setText(tr("Navigation style"));
    _subtitleLabel->setText(tr("How you orbit, pan, and zoom with your mouse"));

    for (int i = 0; i < numFeatured; ++i) {
        _cards[static_cast<size_t>(i)]->setTitle(tr(featuredNames[i]));
    }
}
