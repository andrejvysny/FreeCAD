// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#include "StoryRegistry.h"

namespace FcGallery
{

StoryRegistry& StoryRegistry::instance()
{
    static StoryRegistry registry;
    return registry;
}

void StoryRegistry::registerStories(const char* componentName, StoryList stories)
{
    m_stories[componentName] = std::move(stories);
}

StoryList StoryRegistry::storiesFor(const char* componentName) const
{
    auto it = m_stories.find(componentName);
    if (it != m_stories.end()) {
        return it->second;
    }
    return {};
}

}  // namespace FcGallery
