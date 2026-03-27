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

#include <algorithm>

#include <QApplication>
#include <QContextMenuEvent>
#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLayout>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QShortcutEvent>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QWidget>

#include "StartupWizardOverlay.h"
#include "FirstStartWidget.h"

#include <gsl/pointers>

using namespace StartGui;

namespace
{

constexpr int overlayMargin = 24;
constexpr int verticalMargin = 24;

bool matchesStandardShortcut(
    const QKeySequence& sequence,
    QKeySequence::StandardKey standardKey
)
{
    const auto bindings = QKeySequence::keyBindings(standardKey);
    return std::any_of(bindings.cbegin(), bindings.cend(), [&sequence](const QKeySequence& binding) {
        return !binding.isEmpty() && sequence.matches(binding) == QKeySequence::ExactMatch;
    });
}

}  // namespace

StartupWizardOverlay::StartupWizardOverlay(QWidget* parent)
    : QWidget(parent)
    , _scrollArea {nullptr}
    , _panel {nullptr}
    , _firstStartWidget {nullptr}
    , _filtersInstalled {false}
    , _restoreFocusOnHide {true}
{
    setObjectName(QStringLiteral("startupWizardOverlay"));
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_StyledBackground, true);
    setupUi();
}

StartupWizardOverlay::~StartupWizardOverlay()
{
    removeFilters();
}

void StartupWizardOverlay::setupUi()
{
    auto outerLayout = gsl::owner<QVBoxLayout*>(new QVBoxLayout(this));
    outerLayout->setContentsMargins(overlayMargin, overlayMargin, overlayMargin, overlayMargin);
    outerLayout->setSpacing(0);

    _scrollArea = gsl::owner<QScrollArea*>(new QScrollArea(this));
    _scrollArea->setObjectName(QStringLiteral("startupWizardOverlayScrollArea"));
    _scrollArea->setFrameShape(QFrame::NoFrame);
    _scrollArea->setWidgetResizable(true);
    _scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto scrollWidget = gsl::owner<QWidget*>(new QWidget(_scrollArea));
    _scrollArea->setWidget(scrollWidget);

    auto regionLayout = gsl::owner<QVBoxLayout*>(new QVBoxLayout(scrollWidget));
    regionLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    regionLayout->setContentsMargins(overlayMargin, verticalMargin, overlayMargin, verticalMargin);
    regionLayout->setSpacing(0);

    auto centeredRow = gsl::owner<QHBoxLayout*>(new QHBoxLayout());
    centeredRow->setContentsMargins(0, 0, 0, 0);
    centeredRow->setSpacing(0);

    _panel = gsl::owner<QFrame*>(new QFrame(scrollWidget));
    _panel->setObjectName(QStringLiteral("startupWizardPanel"));
    _panel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    auto panelLayout = gsl::owner<QVBoxLayout*>(new QVBoxLayout(_panel));
    panelLayout->setContentsMargins(0, 0, 0, 0);
    panelLayout->setSpacing(0);

    _firstStartWidget = gsl::owner<FirstStartWidget*>(new FirstStartWidget(_panel));
    panelLayout->addWidget(_firstStartWidget);

    centeredRow->addStretch();
    centeredRow->addWidget(_panel);
    centeredRow->addStretch();

    regionLayout->addStretch(1);
    regionLayout->addLayout(centeredRow);
    regionLayout->addStretch(1);

    outerLayout->addWidget(_scrollArea);

    connect(_firstStartWidget, &FirstStartWidget::dismissed, this, &StartupWizardOverlay::dismissed);
    connect(
        _firstStartWidget,
        &FirstStartWidget::advancedSettingsRequested,
        this,
        &StartupWizardOverlay::advancedSettingsRequested
    );
}

void StartupWizardOverlay::refreshFromPreferences()
{
    if (_firstStartWidget) {
        _firstStartWidget->refreshFromPreferences();
    }
}

void StartupWizardOverlay::showOverlay()
{
    if (auto* window = parentWidget()) {
        _previousFocusWidget = window->focusWidget();
    }

    _restoreFocusOnHide = true;
    refreshFromPreferences();
    syncGeometryAndRaise();
    show();
    raise();

    if (auto* target = focusTarget()) {
        target->setFocus(Qt::OtherFocusReason);
    }
    else {
        setFocus(Qt::OtherFocusReason);
    }
}

void StartupWizardOverlay::hideOverlay(bool restoreFocus)
{
    _restoreFocusOnHide = restoreFocus;
    hide();
}

void StartupWizardOverlay::changeEvent(QEvent* event)
{
    if ((event->type() == QEvent::PaletteChange || event->type() == QEvent::StyleChange) && isVisible()) {
        syncGeometryAndRaise();
    }

    QWidget::changeEvent(event);
}

bool StartupWizardOverlay::eventFilter(QObject* object, QEvent* event)
{
    auto* window = parentWidget();
    if (!window) {
        return QWidget::eventFilter(object, event);
    }

    auto* mainWindow = qobject_cast<QMainWindow*>(window);
    auto* centralWidget = mainWindow ? mainWindow->centralWidget() : nullptr;

    if ((object == window || object == centralWidget)
        && (event->type() == QEvent::Resize || event->type() == QEvent::Move
            || event->type() == QEvent::Show || event->type() == QEvent::ZOrderChange
            || event->type() == QEvent::WindowStateChange
            || event->type() == QEvent::LayoutRequest)) {
        syncGeometryAndRaise();
    }

    if (!isVisible()) {
        return QWidget::eventFilter(object, event);
    }

    if (event->type() == QEvent::ShortcutOverride) {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        if (belongsToMainWindow(object) && !isAllowedShortcut(keyEvent)) {
            keyEvent->accept();
            return true;
        }
    }

    if (event->type() == QEvent::Shortcut) {
        auto* shortcutEvent = static_cast<QShortcutEvent*>(event);
        if (belongsToMainWindow(object) && !isAllowedShortcut(shortcutEvent)) {
            return true;
        }
    }

    return QWidget::eventFilter(object, event);
}

void StartupWizardOverlay::showEvent(QShowEvent* event)
{
    installFilters();
    syncGeometryAndRaise();
    QWidget::showEvent(event);
}

void StartupWizardOverlay::hideEvent(QHideEvent* event)
{
    removeFilters();
    QWidget::hideEvent(event);

    if (_restoreFocusOnHide && _previousFocusWidget && _previousFocusWidget->isVisible()
        && _previousFocusWidget->isEnabled()) {
        _previousFocusWidget->setFocus(Qt::OtherFocusReason);
    }
    else if (_restoreFocusOnHide) {
        if (auto* window = parentWidget()) {
            window->setFocus(Qt::OtherFocusReason);
        }
    }

    _previousFocusWidget.clear();
    _restoreFocusOnHide = true;
}

void StartupWizardOverlay::mousePressEvent(QMouseEvent* event)
{
    event->accept();
}

void StartupWizardOverlay::mouseReleaseEvent(QMouseEvent* event)
{
    event->accept();
}

void StartupWizardOverlay::mouseDoubleClickEvent(QMouseEvent* event)
{
    event->accept();
}

void StartupWizardOverlay::wheelEvent(QWheelEvent* event)
{
    event->accept();
}

void StartupWizardOverlay::contextMenuEvent(QContextMenuEvent* event)
{
    event->accept();
}

void StartupWizardOverlay::keyPressEvent(QKeyEvent* event)
{
    if (isAllowedShortcut(event)) {
        event->ignore();
        return;
    }

    event->accept();
}

void StartupWizardOverlay::keyReleaseEvent(QKeyEvent* event)
{
    if (isAllowedShortcut(event)) {
        event->ignore();
        return;
    }

    event->accept();
}

void StartupWizardOverlay::installFilters()
{
    if (_filtersInstalled) {
        return;
    }

    if (qApp) {
        qApp->installEventFilter(this);
    }
    _filtersInstalled = true;
}

void StartupWizardOverlay::removeFilters()
{
    if (!_filtersInstalled) {
        return;
    }

    if (qApp) {
        qApp->removeEventFilter(this);
    }
    _filtersInstalled = false;
}

void StartupWizardOverlay::syncGeometryAndRaise()
{
    if (auto* window = parentWidget()) {
        setGeometry(window->rect());
        raise();
    }
}

bool StartupWizardOverlay::isAllowedShortcut(const QKeyEvent* event) const
{
    if (!event) {
        return false;
    }

    return isAllowedShortcut(QKeySequence(event->keyCombination()));
}

bool StartupWizardOverlay::isAllowedShortcut(const QShortcutEvent* event) const
{
    if (!event) {
        return false;
    }

    return isAllowedShortcut(event->key());
}

bool StartupWizardOverlay::isAllowedShortcut(const QKeySequence& sequence) const
{
    if (sequence.isEmpty()) {
        return false;
    }

    if (matchesStandardShortcut(sequence, QKeySequence::Quit)
        || matchesStandardShortcut(sequence, QKeySequence::Preferences)
        || matchesStandardShortcut(sequence, QKeySequence::Close)) {
        return true;
    }

#ifdef FC_OS_MACOSX
    if (sequence.matches(QKeySequence(QKeyCombination(Qt::MetaModifier, Qt::Key_H)))
            == QKeySequence::ExactMatch
        || sequence.matches(QKeySequence(QKeyCombination(Qt::MetaModifier, Qt::Key_M)))
            == QKeySequence::ExactMatch
        || sequence.matches(QKeySequence(QKeyCombination(Qt::MetaModifier | Qt::AltModifier, Qt::Key_H)))
            == QKeySequence::ExactMatch) {
        return true;
    }
#endif

    return false;
}

bool StartupWizardOverlay::belongsToMainWindow(const QObject* object) const
{
    auto* window = parentWidget();
    for (auto current = object; current != nullptr; current = current->parent()) {
        if (current == window) {
            return true;
        }
    }

    return false;
}

QWidget* StartupWizardOverlay::focusTarget() const
{
    if (!_firstStartWidget) {
        return nullptr;
    }

    return _firstStartWidget->findChild<QPushButton*>(QStringLiteral("firstStartPrimaryButton"));
}
