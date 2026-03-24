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

#pragma once

#include <QWidget>
#include <array>

class QFrame;
class QLabel;
class QPushButton;

namespace StartGui
{

/// Step 3 of the setup wizard: Community telemetry opt-in page.
/// Allows the user to enable anonymous usage statistics (installed as addon).
class WizardCommunityPage: public QWidget
{
    Q_OBJECT

public:
    explicit WizardCommunityPage(QWidget* parent = nullptr);
    void retranslateUi();
    void applyThemeColors();
    bool isTelemetryEnabled() const;

private:
    void onEnableClicked();
    void updateButtonStyle();

    QLabel* _iconLabel = nullptr;
    QLabel* _headingLabel = nullptr;
    QLabel* _descriptionLabel = nullptr;

    static constexpr int numDataItems = 6;
    std::array<QLabel*, numDataItems> _dataItems {};

    QPushButton* _enableButton = nullptr;
    QFrame* _infoBar = nullptr;
    QLabel* _infoBarLabel = nullptr;
    bool _telemetryEnabled = false;
};

}  // namespace StartGui
