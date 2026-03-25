// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#include "GalleryWindow.h"

#include <QSplitter>

#include "ThemeSwitcher.h"

namespace FcGallery
{

GalleryWindow::GalleryWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("FreeCAD Component Gallery");
    resize(1200, 800);

    setupPanels();
    connectSignals();
}

void GalleryWindow::setupPanels()
{
    // Left: ComponentBrowser
    m_browser = new ComponentBrowser(this);
    m_browser->setMinimumWidth(180);

    // Center top: ComponentCanvas with inline theme switcher
    m_canvas = new ComponentCanvas(this);
    auto* themeSwitcher = new ThemeSwitcher(this);
    m_canvas->setThemeSwitcher(themeSwitcher);

    // Center bottom: StoryPanel
    m_storyPanel = new StoryPanel(this);

    // Center vertical splitter
    auto* centerSplitter = new QSplitter(Qt::Vertical, this);
    centerSplitter->addWidget(m_canvas);
    centerSplitter->addWidget(m_storyPanel);
    centerSplitter->setStretchFactor(0, 1);
    centerSplitter->setStretchFactor(1, 0);
    centerSplitter->setChildrenCollapsible(false);
    m_storyPanel->setMaximumHeight(80);

    // Right top: PropertyPanel
    m_propertyPanel = new PropertyPanel(this);

    // Right bottom: CodePreview
    m_codePreview = new CodePreview(this);

    // Right vertical splitter
    auto* rightSplitter = new QSplitter(Qt::Vertical, this);
    rightSplitter->addWidget(m_propertyPanel);
    rightSplitter->addWidget(m_codePreview);
    rightSplitter->setStretchFactor(0, 1);
    rightSplitter->setStretchFactor(1, 1);

    // Main horizontal splitter
    auto* mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->addWidget(m_browser);
    mainSplitter->addWidget(centerSplitter);
    mainSplitter->addWidget(rightSplitter);
    mainSplitter->setSizes({220, 580, 300});
    mainSplitter->setStyleSheet(
        "QSplitter::handle { background: palette(midlight); }"
        "QSplitter::handle:horizontal { width: 1px; }"
        "QSplitter::handle:vertical { height: 1px; }");

    setCentralWidget(mainSplitter);
}

void GalleryWindow::connectSignals()
{
    connect(m_browser, &ComponentBrowser::componentSelected, this,
            &GalleryWindow::onComponentSelected);

    connect(m_storyPanel, &StoryPanel::storyApplied, this, &GalleryWindow::onStoryApplied);
}

void GalleryWindow::onComponentSelected(const FcComponents::ComponentInfo& info)
{
    m_currentInfo = info;
    m_hasCurrentInfo = true;

    m_canvas->setComponent(info);

    QWidget* widget = m_canvas->currentWidget();
    m_storyPanel->setComponent(info.name, widget);
    m_codePreview->setComponent(info);

    // After story panel applies default story, refresh property panel and code
    if (widget) {
        m_propertyPanel->setWidget(widget, info.metaObject);
        m_codePreview->refresh(info, widget, info.metaObject);
    }
}

void GalleryWindow::onStoryApplied()
{
    QWidget* widget = m_canvas->currentWidget();
    if (!widget || !m_hasCurrentInfo) {
        return;
    }

    m_propertyPanel->setWidget(widget, m_currentInfo.metaObject);
    m_codePreview->refresh(m_currentInfo, widget, m_currentInfo.metaObject);
}

}  // namespace FcGallery
