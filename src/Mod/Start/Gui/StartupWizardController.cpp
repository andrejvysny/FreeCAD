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

#include "PreCompiled.h"

#include <QApplication>
#include <QTimer>
#include <QWidget>

#include "StartupWizardController.h"
#include "StartupWizardOverlay.h"

#include <App/Application.h>
#include <Gui/Application.h>
#include <Gui/Command.h>
#include <Gui/MainWindow.h>

using namespace StartGui;

namespace
{

constexpr int startupRetryDelayMs = 100;

}  // namespace

StartupWizardController::StartupWizardController(QObject* parent)
    : QObject(parent)
    , _overlay {nullptr}
    , _startupScheduled {false}
    , _startupHandled {false}
    , _startPageLaunchRequested {false}
{}

StartupWizardController& StartupWizardController::instance()
{
    static auto* controller = new StartupWizardController(qApp);
    return *controller;
}

void StartupWizardController::scheduleStartup()
{
    if (_startupHandled || _startupScheduled) {
        return;
    }

    _startupScheduled = true;
    QTimer::singleShot(startupRetryDelayMs, this, [this]() { attemptStartup(); });
}

void StartupWizardController::showIfPending()
{
    if (!isFirstStartPending()) {
        return;
    }

    ensureOverlay();
    if (_overlay) {
        _overlay->showOverlay();
    }
}

void StartupWizardController::showManual()
{
    if (hasBlockingModalWidget()) {
        return;
    }

    ensureOverlay();
    if (_overlay) {
        _overlay->showOverlay();
    }
}

StartupWizardOverlay* StartupWizardController::overlay() const
{
    return _overlay;
}

void StartupWizardController::attemptStartup()
{
    _startupScheduled = false;
    if (_startupHandled) {
        return;
    }

    if (!isStartupReady()) {
        scheduleStartup();
        return;
    }

    if (shouldLaunchStartPage() && !_startPageLaunchRequested) {
        if (!runCommandByName("Start_Start")) {
            scheduleStartup();
            return;
        }

        _startPageLaunchRequested = true;
        QTimer::singleShot(0, this, [this]() { attemptStartup(); });
        return;
    }

    if (!isStartupReady()) {
        scheduleStartup();
        return;
    }

    showIfPending();
    _startupHandled = true;
}

QWidget* StartupWizardController::getMainWindow() const
{
    return Gui::getMainWindow();
}

bool StartupWizardController::canRunCommandByName(const char* name) const
{
    return Gui::Application::Instance
        && Gui::Application::Instance->commandManager().getCommandByName(name);
}

bool StartupWizardController::runCommandByName(const char* name)
{
    if (!canRunCommandByName(name)) {
        return false;
    }

    Gui::Application::Instance->commandManager().runCommandByName(name);
    return true;
}

bool StartupWizardController::isStartupReady() const
{
    auto* mainWindow = getMainWindow();
    if (!mainWindow || !mainWindow->isVisible()) {
        return false;
    }

    if (!mainWindow->property("eventLoop").toBool()) {
        return false;
    }

    return !hasBlockingModalWidget();
}

bool StartupWizardController::hasBlockingModalWidget() const
{
    auto* activeModal = qApp->activeModalWidget();
    return activeModal != nullptr && activeModal != _overlay;
}

void StartupWizardController::ensureOverlay()
{
    auto* mainWindow = getMainWindow();
    if (!mainWindow) {
        return;
    }

    if (_overlay && _overlay->parentWidget() == mainWindow) {
        return;
    }

    if (_overlay) {
        _overlay->deleteLater();
        _overlay.clear();
    }

    _overlay = new StartupWizardOverlay(mainWindow);
    connect(_overlay, &StartupWizardOverlay::dismissed, this, &StartupWizardController::onOverlayDismissed);
    connect(
        _overlay,
        &StartupWizardOverlay::advancedSettingsRequested,
        this,
        &StartupWizardController::onAdvancedSettingsRequested
    );
}

bool StartupWizardController::shouldLaunchStartPage() const
{
    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Mod/Start"
    );
    return hGrp->GetBool("ShowOnStartup", true);
}

bool StartupWizardController::isFirstStartPending() const
{
    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Mod/Start"
    );
    return hGrp->GetBool("FirstStart2024", true);
}

void StartupWizardController::markFirstStartCompleted() const
{
    auto hGrp = App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/Mod/Start"
    );
    hGrp->SetBool("FirstStart2024", false);
}

void StartupWizardController::onOverlayDismissed()
{
    markFirstStartCompleted();
    if (_overlay) {
        _overlay->hideOverlay();
    }
}

void StartupWizardController::onAdvancedSettingsRequested()
{
    if (_overlay) {
        _overlay->hideOverlay(false);
    }

    runCommandByName("Std_DlgPreferences");
}
