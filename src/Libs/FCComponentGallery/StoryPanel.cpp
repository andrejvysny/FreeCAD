// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#include "StoryPanel.h"

#include <QFrame>
#include <QLabel>
#include <QPushButton>

#include "FlowLayout.h"
#include "Stories/StoryRegistry.h"

namespace FcGallery
{

namespace
{

const char* pillStyle =
    "QPushButton { border: 1px solid palette(mid); border-radius: 14px; "
    "padding: 4px 16px; background: transparent; color: palette(window-text); "
    "font-size: 11px; }"
    "QPushButton:checked { background: palette(highlight); "
    "color: palette(highlighted-text); border-color: palette(highlight); }"
    "QPushButton:hover:!checked { background: palette(alternate-base); }";

}  // namespace

StoryPanel::StoryPanel(QWidget* parent)
    : QWidget(parent)
{
    m_outerLayout = new QVBoxLayout(this);
    m_outerLayout->setContentsMargins(0, 0, 0, 0);
    m_outerLayout->setSpacing(0);

    // Thin separator line
    auto* separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Plain);
    separator->setStyleSheet("QFrame { color: palette(midlight); max-height: 1px; }");
    m_outerLayout->addWidget(separator);

    // Pill container with flow layout
    m_pillContainer = new QWidget(this);
    m_flowLayout = new FlowLayout(m_pillContainer, -1, 6, 6);
    m_flowLayout->setContentsMargins(8, 6, 8, 6);
    m_outerLayout->addWidget(m_pillContainer);

    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);
}

void StoryPanel::setComponent(const char* componentName, QWidget* targetWidget)
{
    clearStories();
    m_target = targetWidget;
    m_stories = StoryRegistry::instance().storiesFor(componentName);

    if (m_stories.empty()) {
        auto* label = new QLabel("No stories", m_pillContainer);
        label->setStyleSheet("color: palette(placeholder-text); font-style: italic; padding: 4px;");
        m_flowLayout->addWidget(label);
        return;
    }

    for (int i = 0; i < static_cast<int>(m_stories.size()); ++i) {
        const auto& story = m_stories[static_cast<size_t>(i)];

        auto* pill = new QPushButton(story.name, m_pillContainer);
        pill->setCheckable(true);
        pill->setCursor(Qt::PointingHandCursor);
        pill->setStyleSheet(pillStyle);
        pill->setToolTip(story.description);

        m_group->addButton(pill, i);
        m_flowLayout->addWidget(pill);
    }

    connect(m_group, &QButtonGroup::idClicked, this, [this](int id) {
        if (id < 0 || id >= static_cast<int>(m_stories.size()) || !m_target) {
            return;
        }
        m_stories[static_cast<size_t>(id)].configure(m_target);
        Q_EMIT storyApplied();
    });

    // Select first story by default
    if (auto* first = m_group->button(0)) {
        first->setChecked(true);
        if (m_target && !m_stories.empty()) {
            m_stories[0].configure(m_target);
        }
    }
}

void StoryPanel::clearStories()
{
    // Remove all buttons from the group
    for (auto* btn : m_group->buttons()) {
        m_group->removeButton(btn);
    }

    // Remove all widgets from the flow layout
    while (m_flowLayout->count() > 0) {
        QLayoutItem* item = m_flowLayout->takeAt(0);
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    m_stories.clear();
    m_target = nullptr;
}

}  // namespace FcGallery
