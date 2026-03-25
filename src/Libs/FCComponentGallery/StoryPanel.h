// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#pragma once

#include <QButtonGroup>
#include <QVBoxLayout>
#include <QWidget>

#include "Stories/Story.h"

namespace FcGallery
{

class FlowLayout;

/// Panel below the canvas: horizontal pill buttons for selecting story configurations.
class StoryPanel : public QWidget
{
    Q_OBJECT

public:
    explicit StoryPanel(QWidget* parent = nullptr);

    /// Load stories for the named component and wire them to the target widget.
    void setComponent(const char* componentName, QWidget* targetWidget);

Q_SIGNALS:
    void storyApplied();

private:
    void clearStories();

    QVBoxLayout* m_outerLayout = nullptr;
    QWidget* m_pillContainer = nullptr;
    FlowLayout* m_flowLayout = nullptr;
    QButtonGroup* m_group = nullptr;
    StoryList m_stories;
    QWidget* m_target = nullptr;
};

}  // namespace FcGallery
