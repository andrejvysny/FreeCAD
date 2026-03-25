// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#pragma once

#include <QMainWindow>

#include <FCComponentLib/Components/ComponentMeta.h>

#include "CodePreview.h"
#include "ComponentBrowser.h"
#include "ComponentCanvas.h"
#include "PropertyPanel.h"
#include "StoryPanel.h"

namespace FcGallery
{

/// Main window: three-panel layout with browser, canvas+stories, properties+code.
class GalleryWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GalleryWindow(QWidget* parent = nullptr);

private Q_SLOTS:
    void onComponentSelected(const FcComponents::ComponentInfo& info);
    void onStoryApplied();

private:
    void setupPanels();
    void connectSignals();

    ComponentBrowser* m_browser = nullptr;
    ComponentCanvas* m_canvas = nullptr;
    StoryPanel* m_storyPanel = nullptr;
    PropertyPanel* m_propertyPanel = nullptr;
    CodePreview* m_codePreview = nullptr;

    // Cache current component info for refresh after story apply
    FcComponents::ComponentInfo m_currentInfo {};
    bool m_hasCurrentInfo = false;
};

}  // namespace FcGallery
