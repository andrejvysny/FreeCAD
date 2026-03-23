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

#include "RecentFilesModel.h"
#include <App/Application.h>
#include <App/ProjectFile.h>

using namespace Start;

RecentFilesModel::RecentFilesModel(QObject* parent)
    : DisplayedFilesModel(parent)
{
    _parameterGroup = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/RecentFiles"
    );
    _startParameterGroup = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Mod/Start"
    );
    loadPinnedFiles();
}

QVariant RecentFilesModel::data(const QModelIndex& index, int role) const
{
    if (role == static_cast<int>(DisplayedFilesModelRoles::pinned)) {
        auto path = DisplayedFilesModel::data(index, static_cast<int>(DisplayedFilesModelRoles::path)).toString();
        return _pinnedPaths.contains(path);
    }
    return DisplayedFilesModel::data(index, role);
}

void RecentFilesModel::loadRecentFiles()
{
    beginResetModel();
    clear();

    // Collect all file paths, separating pinned from unpinned
    QStringList pinnedFiles;
    QStringList unpinnedFiles;

    auto numRows {_parameterGroup->GetInt("RecentFiles", 0)};
    for (int i = 0; i < numRows; ++i) {
        auto entry = fmt::format("MRU{}", i);
        auto path = QString::fromStdString(_parameterGroup->GetASCII(entry.c_str(), ""));
        if (_pinnedPaths.contains(path)) {
            pinnedFiles.append(path);
        }
        else {
            unpinnedFiles.append(path);
        }
    }

    // Add pinned files first, then unpinned
    for (const auto& path : pinnedFiles) {
        addFile(path);
    }
    for (const auto& path : unpinnedFiles) {
        addFile(path);
    }

    endResetModel();
}

void RecentFilesModel::recentFileAdded(const QString& filename)
{
    Q_UNUSED(filename)
    loadRecentFiles();
}

bool RecentFilesModel::isPinned(const QString& path) const
{
    return _pinnedPaths.contains(path);
}

void RecentFilesModel::togglePinned(const QString& path)
{
    if (_pinnedPaths.contains(path)) {
        _pinnedPaths.remove(path);
    }
    else {
        _pinnedPaths.insert(path);
    }
    savePinnedFiles();
    loadRecentFiles();
}

void RecentFilesModel::loadPinnedFiles()
{
    _pinnedPaths.clear();
    auto pinnedGrp = _startParameterGroup->GetGroup("PinnedFiles");
    auto numPinned = pinnedGrp->GetInt("Count", 0);
    for (int i = 0; i < numPinned; ++i) {
        auto entry = fmt::format("File{}", i);
        auto path = pinnedGrp->GetASCII(entry.c_str(), "");
        if (!path.empty()) {
            _pinnedPaths.insert(QString::fromStdString(path));
        }
    }
}

void RecentFilesModel::savePinnedFiles()
{
    auto pinnedGrp = _startParameterGroup->GetGroup("PinnedFiles");
    pinnedGrp->Clear();
    pinnedGrp->SetInt("Count", static_cast<long>(_pinnedPaths.size()));
    int i = 0;
    for (const auto& path : _pinnedPaths) {
        auto entry = fmt::format("File{}", i);
        pinnedGrp->SetASCII(entry.c_str(), path.toStdString().c_str());
        ++i;
    }
}
