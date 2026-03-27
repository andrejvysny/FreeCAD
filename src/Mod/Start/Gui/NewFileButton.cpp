// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
 *   Copyright (c) 2025 Alfredo Monclus <alfredomonclus@gmail.com>          *
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

#include <QCursor>
#include <QLabel>
#include <QFont>
#include <QIcon>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QString>

#include "NewFileButton.h"
#include <algorithm>

namespace StartGui
{

NewFileButton::NewFileButton(const NewButton& newButton, bool compact)
    : isCompact(compact)
    , mainLayout(new QHBoxLayout(this))
    , textLayout(new QVBoxLayout())
    , headingLabel(new QLabel())
    , descriptionLabel(new QLabel())
{
    setObjectName(QStringLiteral("newFileButton"));
    setCursor(Qt::PointingHandCursor);
    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Mod/Start"
    );

    constexpr int defaultWidth = 180;  // #newFileButton width in QSS
    labelWidth = int(hGrp->GetInt("FileCardLabelWith", defaultWidth));

    constexpr int defaultSize = 48;
    constexpr int compactIconSize = 32;
    iconSize = isCompact ? compactIconSize : int(hGrp->GetInt("NewFileIconSize", defaultSize));

    auto iconLabel = new QLabel(this);
    QIcon baseIcon(newButton.iconPath);
    iconLabel->setPixmap(baseIcon.pixmap(iconSize, iconSize));

    textLayout->addWidget(headingLabel);
    textLayout->addWidget(descriptionLabel);
    textLayout->setSpacing(0);
    textLayout->setContentsMargins(0, 0, 0, 0);

    headingLabel->setText(newButton.heading);
    QFont font = headingLabel->font();
    font.setWeight(QFont::Bold);
    headingLabel->setFont(font);

    descriptionLabel->setText(newButton.description);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setFixedWidth(labelWidth);
    descriptionLabel->setAlignment(Qt::AlignTop);

    mainLayout->setAlignment(Qt::AlignVCenter);
    mainLayout->addWidget(iconLabel);
    mainLayout->addLayout(textLayout);
    mainLayout->addStretch();

    if (isCompact) {
        descriptionLabel->hide();
        mainLayout->setSpacing(8);
        mainLayout->setContentsMargins(10, 8, 16, 8);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }
    else {
        QFontMetrics qfm(font);
        int margin = qfm.height() / 2;
        mainLayout->setSpacing(margin);
        mainLayout->setContentsMargins(margin, margin, 2 * margin, margin);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    }

    setLayout(mainLayout);
}

QSize NewFileButton::minimumSizeHint() const
{
    if (isCompact) {
        int minWidth = iconSize + headingLabel->sizeHint().width()
            + mainLayout->contentsMargins().left() + mainLayout->contentsMargins().right()
            + mainLayout->spacing();

        int minHeight = std::max(iconSize, headingLabel->sizeHint().height())
            + mainLayout->contentsMargins().top() + mainLayout->contentsMargins().bottom();

        return {minWidth, minHeight};
    }

    int minWidth = labelWidth + iconSize + mainLayout->contentsMargins().left()
        + mainLayout->contentsMargins().right() + mainLayout->spacing();

    int textHeight = headingLabel->sizeHint().height() + descriptionLabel->sizeHint().height();

    int minHeight = std::max(iconSize, textHeight) + mainLayout->contentsMargins().top()
        + mainLayout->contentsMargins().bottom();

    return {minWidth, minHeight};
}

}  // namespace StartGui
