// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
 *   Copyright (c) 2026 The FreeCAD Project Association AISBL               *
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

#include <Mod/Start/StartGlobal.h>

#include <QPointer>
#include <QWidget>

class QContextMenuEvent;
class QFrame;
class QHideEvent;
class QKeyEvent;
class QMouseEvent;
class QScrollArea;
class QShowEvent;
class QWheelEvent;

namespace StartGui
{

class FirstStartWidget;

class StartGuiExport StartupWizardOverlay: public QWidget
{
    Q_OBJECT

public:
    explicit StartupWizardOverlay(QWidget* parent = nullptr);

    void refreshFromPreferences();
    void showOverlay();

Q_SIGNALS:
    void dismissed();
    void advancedSettingsRequested();

protected:
    bool eventFilter(QObject* object, QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private:
    void setupUi();
    void syncGeometryAndRaise();
    bool belongsToMainWindow(const QObject* object) const;
    bool belongsToOverlay(const QObject* object) const;
    QWidget* focusTarget() const;

    QScrollArea* _scrollArea;
    QFrame* _panel;
    FirstStartWidget* _firstStartWidget;
    QPointer<QWidget> _previousFocusWidget;
};

}  // namespace StartGui
