// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#include <QIcon>
#include <QPushButton>

#include "StoryRegistry.h"

namespace
{

struct ButtonStoriesRegistrar
{
    ButtonStoriesRegistrar()
    {
        FcGallery::StoryRegistry::instance().registerStories("FcPushButton", {
            {
                "Default",
                "Button in its default state",
                [](QWidget* w) {
                    auto* btn = qobject_cast<QPushButton*>(w);
                    if (!btn) {
                        return;
                    }
                    btn->setText("Click Me");
                    btn->setEnabled(true);
                    btn->setFlat(false);
                    btn->setIcon(QIcon());
                }
            },
        });
    }
};

static ButtonStoriesRegistrar s_buttonStories;

}  // namespace
