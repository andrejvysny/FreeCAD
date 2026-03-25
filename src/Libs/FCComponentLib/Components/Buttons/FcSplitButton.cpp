// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
 *   Copyright (c) 2025 FreeCAD Project Association                         *
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

#include "FcSplitButton.h"

#include <QHBoxLayout>

#include <FCComponentLib/Components/ComponentMeta.h>
#include <FCComponentLib/Components/ComponentRegistry.h>

namespace FcComponents
{

FcSplitButton::FcSplitButton(QWidget* parent)
    : FcSplitButton(QStringLiteral(""), parent)
{}

FcSplitButton::FcSplitButton(const QString& text, QWidget* parent)
    : QWidget(parent)
    , m_main(new QPushButton(text, this))
    , m_menuButton(new QToolButton(this))
    , m_menu(new QMenu(this))
{
    auto* layout = new QHBoxLayout(this);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_main);
    layout->addWidget(m_menuButton);

    // Behavior
    m_main->setAutoDefault(false);
    m_main->setDefault(false);

    m_menuButton->setMenu(m_menu);
    m_menuButton->setPopupMode(QToolButton::InstantPopup);
    m_menuButton->setArrowType(Qt::DownArrow);
    m_menuButton->setToolButtonStyle(Qt::ToolButtonIconOnly);

    connect(m_main, &QPushButton::clicked, this, &FcSplitButton::defaultClicked);
    connect(m_menu, &QMenu::triggered, this, &FcSplitButton::triggered);

    // Styling to make it look like a single split button
    m_main->setProperty("splitRole", QLatin1String("main"));
    m_menuButton->setProperty("splitRole", QLatin1String("menu"));
}

QPushButton* FcSplitButton::mainButton() const
{
    return m_main;
}

QToolButton* FcSplitButton::menuButton() const
{
    return m_menuButton;
}

QMenu* FcSplitButton::menu() const
{
    return m_menu;
}

}  // namespace FcComponents

FC_REGISTER_COMPONENT(FcSplitButton, "Buttons", "Split button with dropdown menu")
