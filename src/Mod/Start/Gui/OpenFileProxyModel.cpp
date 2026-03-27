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

#include "OpenFileProxyModel.h"

#include "../App/DisplayedFilesModel.h"

namespace StartGui
{

const QString OpenFileProxyModel::OpenFileSentinel = QStringLiteral("__open_file_dialog__");

OpenFileProxyModel::OpenFileProxyModel(QObject* parent)
    : QIdentityProxyModel(parent)
{}

int OpenFileProxyModel::rowCount(const QModelIndex& parent) const
{
    if (!sourceModel() || parent.isValid()) {
        return 0;
    }
    return sourceModel()->rowCount() + 1;
}

QVariant OpenFileProxyModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    // Delegate all non-virtual rows to the source model
    if (index.row() < rowCount() - 1) {
        return QIdentityProxyModel::data(index, role);
    }

    // Virtual "Open file..." row
    auto typedRole = static_cast<Start::DisplayedFilesModelRoles>(role);
    switch (typedRole) {
        case Start::DisplayedFilesModelRoles::baseName:
            return tr("Open file...");
        case Start::DisplayedFilesModelRoles::path:
            return OpenFileSentinel;
        case Start::DisplayedFilesModelRoles::image:
            return QByteArray();
        case Start::DisplayedFilesModelRoles::size:
            return QString();
        default:
            return {};
    }
}

Qt::ItemFlags OpenFileProxyModel::flags(const QModelIndex& index) const
{
    if (index.isValid() && index.row() == rowCount() - 1) {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
    return QIdentityProxyModel::flags(index);
}

QModelIndex OpenFileProxyModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid() || !sourceModel()) {
        return {};
    }

    // Virtual row: create an index that has no source counterpart
    if (row == sourceModel()->rowCount()) {
        return createIndex(row, column);
    }

    return QIdentityProxyModel::index(row, column, parent);
}

QModelIndex OpenFileProxyModel::parent(const QModelIndex& /*index*/) const
{
    // Flat list — no item has a parent
    return {};
}

}  // namespace StartGui
