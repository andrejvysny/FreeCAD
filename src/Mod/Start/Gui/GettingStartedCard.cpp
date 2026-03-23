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
#include <QLabel>
#include <QVBoxLayout>

#include "GettingStartedCard.h"

using namespace StartGui;

GettingStartedCard::GettingStartedCard(QWidget* parent)
    : QFrame(parent)
    , _titleLabel(new QLabel(this))
    , _bodyLabel(new QLabel(this))
{
    setObjectName(QStringLiteral("gettingStartedCard"));

    auto* layout = new QVBoxLayout(this);

    _titleLabel->setObjectName(QStringLiteral("startSectionLabel"));
    QFont titleFont = _titleLabel->font();
    titleFont.setBold(true);
    _titleLabel->setFont(titleFont);
    _titleLabel->setText(tr("GETTING STARTED"));

    _bodyLabel->setWordWrap(true);
    _bodyLabel->setOpenExternalLinks(true);
    _bodyLabel->setTextFormat(Qt::RichText);
    _bodyLabel->setText(
        tr("New to FreeCAD? Start by creating a <b>Parametric body</b>, then add a sketch to a "
           "face. The <a href=\"https://wiki.freecad.org/Getting_started\">beginner tutorial</a> "
           "walks you through your first part in 5 minutes.")
    );

    layout->addWidget(_titleLabel);
    layout->addWidget(_bodyLabel);
}
