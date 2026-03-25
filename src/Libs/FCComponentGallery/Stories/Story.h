// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#pragma once

#include <functional>
#include <vector>

#include <QString>
#include <QWidget>

namespace FcGallery
{

/// A single "story" — a named configuration applied to a live widget instance.
struct Story
{
    QString name;
    QString description;
    std::function<void(QWidget*)> configure;
};

using StoryList = std::vector<Story>;

}  // namespace FcGallery
