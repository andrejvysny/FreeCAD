// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#include <QMenu>
#include <QPushButton>
#include <QStyle>
#include <QToolButton>

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
            {
                "Disabled",
                "Button with interaction disabled",
                [](QWidget* w) {
                    auto* btn = qobject_cast<QPushButton*>(w);
                    if (!btn) {
                        return;
                    }
                    btn->setText("Disabled");
                    btn->setEnabled(false);
                }
            },
            {
                "Flat",
                "Flat button without raised border",
                [](QWidget* w) {
                    auto* btn = qobject_cast<QPushButton*>(w);
                    if (!btn) {
                        return;
                    }
                    btn->setText("Flat Button");
                    btn->setFlat(true);
                    btn->setEnabled(true);
                }
            },
            {
                "With Icon",
                "Button displaying a file icon",
                [](QWidget* w) {
                    auto* btn = qobject_cast<QPushButton*>(w);
                    if (!btn) {
                        return;
                    }
                    btn->setText("Open File");
                    btn->setIcon(w->style()->standardIcon(QStyle::SP_FileIcon));
                    btn->setEnabled(true);
                    btn->setFlat(false);
                }
            },
        });
    }
};

static ButtonStoriesRegistrar s_buttonStories;


struct SplitButtonStoriesRegistrar
{
    SplitButtonStoriesRegistrar()
    {
        FcGallery::StoryRegistry::instance().registerStories("FcSplitButton", {
            {
                "Default",
                "Split button with text label",
                [](QWidget* w) {
                    auto* btn = w->findChild<QPushButton*>();
                    if (btn) {
                        btn->setText("Save");
                    }
                    w->setEnabled(true);
                }
            },
            {
                "With Actions",
                "Split button with populated menu",
                [](QWidget* w) {
                    auto* btn = w->findChild<QPushButton*>();
                    if (btn) {
                        btn->setText("Export");
                    }
                    auto* menu = w->findChild<QMenu*>();
                    if (menu) {
                        menu->clear();
                        menu->addAction("Export as PDF");
                        menu->addAction("Export as SVG");
                        menu->addAction("Export as PNG");
                    }
                }
            },
            {
                "Disabled",
                "Split button with interaction disabled",
                [](QWidget* w) {
                    auto* btn = w->findChild<QPushButton*>();
                    if (btn) {
                        btn->setText("Disabled");
                    }
                    w->setEnabled(false);
                }
            },
        });
    }
};

static SplitButtonStoriesRegistrar s_splitButtonStories;

}  // namespace
