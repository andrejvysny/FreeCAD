// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#pragma once

#include <map>
#include <string>

#include "Story.h"

namespace FcGallery
{

/// Global registry mapping component names to their story lists.
///
/// Stories are registered at static-init time from per-component .cpp files
/// (ButtonStories.cpp, InputStories.cpp, etc.), mirroring how FCComponentLib
/// registers components via FC_REGISTER_COMPONENT.
class StoryRegistry
{
public:
    static StoryRegistry& instance();

    void registerStories(const char* componentName, StoryList stories);
    StoryList storiesFor(const char* componentName) const;

    StoryRegistry(const StoryRegistry&) = delete;
    StoryRegistry& operator=(const StoryRegistry&) = delete;

private:
    StoryRegistry() = default;

    std::map<std::string, StoryList> m_stories;
};

}  // namespace FcGallery
