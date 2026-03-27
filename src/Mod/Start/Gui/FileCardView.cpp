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

#include "FileCardView.h"

#include "../App/DisplayedFilesModel.h"
#include <algorithm>

namespace StartGui
{

FileCardView::FileCardView(QWidget* parent)
    : QListView(parent)
{
    setFrameShape(QFrame::NoFrame);
    setStyleSheet(QStringLiteral("QListView { border: none; background: transparent; }"));
    QSizePolicy sizePolicy(QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::MinimumExpanding);
    sizePolicy.setHeightForWidth(true);
    setSizePolicy(sizePolicy);
    setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    setViewMode(QListView::ViewMode::IconMode);
    setFlow(QListView::Flow::LeftToRight);
    setResizeMode(QListView::ResizeMode::Adjust);
    setUniformItemSizes(true);
    setMouseTracking(true);

    setSpacing(0);
}

int FileCardView::heightForWidth(int width) const
{
    auto model = this->model();
    auto delegate = this->itemDelegate();
    if (!model || !delegate) {
        return 0;
    }
    int numCards = model->rowCount();
    auto cardSize = delegate->sizeHint(QStyleOptionViewItem(), model->index(0, 0));
    int cardsPerRow = std::max(1, static_cast<int>(width / cardSize.width()));
    int numRows = static_cast<int>(
        ceil(static_cast<double>(numCards) / static_cast<double>(cardsPerRow))
    );
    constexpr int extra = 4;  // avoid tiny scrollbars
    return numRows * cardSize.height() + extra;
}

QSize FileCardView::sizeHint() const
{
    auto model = this->model();
    auto delegate = this->itemDelegate();
    if (!model || !delegate) {
        return {16, 16};
    }
    int numCards = model->rowCount();
    auto cardSize = delegate->sizeHint(QStyleOptionViewItem(), model->index(0, 0));
    return {
        cardSize.width() * numCards,
        cardSize.height()
    };
}

}  // namespace StartGui
