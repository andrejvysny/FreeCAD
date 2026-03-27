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
class QEvent;
class QFrame;
class QHideEvent;
class QKeyEvent;
class QKeySequence;
class QMouseEvent;
class QScrollArea;
class QShortcutEvent;
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
    ~StartupWizardOverlay() override;

    void refreshFromPreferences();
    void showOverlay();
    void hideOverlay(bool restoreFocus = true);

Q_SIGNALS:
    void dismissed();
    void advancedSettingsRequested();

protected:
    void changeEvent(QEvent* event) override;
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
    void installFilters();
    void removeFilters();
    void setupUi();
    void syncGeometryAndRaise();
    bool isAllowedShortcut(const QKeyEvent* event) const;
    bool isAllowedShortcut(const QShortcutEvent* event) const;
    bool isAllowedShortcut(const QKeySequence& sequence) const;
    bool belongsToMainWindow(const QObject* object) const;
    QWidget* focusTarget() const;

    QScrollArea* _scrollArea;
    QFrame* _panel;
    FirstStartWidget* _firstStartWidget;
    QPointer<QWidget> _previousFocusWidget;
    bool _filtersInstalled;
    bool _restoreFocusOnHide;
};

}  // namespace StartGui
