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
#include <QObject>

class QWidget;

namespace StartGui
{

class StartupWizardOverlay;

class StartGuiExport StartupWizardController: public QObject
{
    Q_OBJECT

public:
    explicit StartupWizardController(QObject* parent = nullptr);

    static StartupWizardController& instance();

    void scheduleStartup();
    virtual void showIfPending();
    virtual void showManual();

    StartupWizardOverlay* overlay() const;

protected:
    void attemptStartup();
    virtual QWidget* getMainWindow() const;
    virtual bool canRunCommandByName(const char* name) const;
    virtual bool runCommandByName(const char* name);
    virtual bool isStartupReady() const;

private:
    void ensureOverlay();
    bool shouldLaunchStartPage() const;
    bool isFirstStartPending() const;
    void markFirstStartCompleted() const;

private Q_SLOTS:
    void onOverlayDismissed();
    void onAdvancedSettingsRequested();

private:
    QPointer<StartupWizardOverlay> _overlay;
    bool _startupScheduled;
    bool _startupHandled;
};

}  // namespace StartGui
