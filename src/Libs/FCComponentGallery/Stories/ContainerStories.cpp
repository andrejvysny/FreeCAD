// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#include <QTreeWidget>

#include "StoryRegistry.h"

namespace
{

struct ContainerStoriesRegistrar
{
    ContainerStoriesRegistrar()
    {
        // --- FcActionSelector stories ---
        FcGallery::StoryRegistry::instance().registerStories("FcActionSelector", {
            {
                "Default",
                "Action selector with empty panels",
                [](QWidget* w) {
                    w->setEnabled(true);
                    w->setMinimumSize(400, 200);
                }
            },
            {
                "With Items",
                "Action selector with sample items in available panel",
                [](QWidget* w) {
                    // Find the first QTreeWidget (available items panel)
                    auto trees = w->findChildren<QTreeWidget*>();
                    if (trees.size() >= 1) {
                        trees[0]->clear();
                        new QTreeWidgetItem(trees[0], QStringList{"Item A"});
                        new QTreeWidgetItem(trees[0], QStringList{"Item B"});
                        new QTreeWidgetItem(trees[0], QStringList{"Item C"});
                        new QTreeWidgetItem(trees[0], QStringList{"Item D"});
                    }
                    w->setMinimumSize(400, 200);
                }
            },
            {
                "Disabled",
                "Action selector with interaction disabled",
                [](QWidget* w) {
                    w->setEnabled(false);
                    w->setMinimumSize(400, 200);
                }
            },
        });
    }
};

static ContainerStoriesRegistrar s_containerStories;

}  // namespace
