// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#include <QLabel>
#include <QMetaProperty>
#include <QPlainTextEdit>

#include "StoryRegistry.h"

namespace
{

/// Helper: write a Q_PROPERTY by name via the meta-object system.
void setMetaProperty(QWidget* w, const char* name, const QVariant& value)
{
    const QMetaObject* meta = w->metaObject();
    int idx = meta->indexOfProperty(name);
    if (idx >= 0) {
        meta->property(idx).write(w, value);
    }
}

struct DisplayStoriesRegistrar
{
    DisplayStoriesRegistrar()
    {
        // --- FcUrlLabel stories ---
        FcGallery::StoryRegistry::instance().registerStories("FcUrlLabel", {
            {
                "Default",
                "URL label linking to FreeCAD website",
                [](QWidget* w) {
                    auto* lbl = qobject_cast<QLabel*>(w);
                    if (!lbl) {
                        return;
                    }
                    lbl->setText("FreeCAD Website");
                    setMetaProperty(w, "url", QStringLiteral("https://www.freecad.org"));
                    setMetaProperty(w, "launchExternal", true);
                    lbl->setEnabled(true);
                }
            },
            {
                "Internal Link",
                "URL label that emits signal instead of opening browser",
                [](QWidget* w) {
                    auto* lbl = qobject_cast<QLabel*>(w);
                    if (!lbl) {
                        return;
                    }
                    lbl->setText("Internal Action");
                    setMetaProperty(w, "url", QStringLiteral("internal://action"));
                    setMetaProperty(w, "launchExternal", false);
                }
            },
            {
                "Disabled",
                "URL label with interaction disabled",
                [](QWidget* w) {
                    auto* lbl = qobject_cast<QLabel*>(w);
                    if (!lbl) {
                        return;
                    }
                    lbl->setText("Disabled Link");
                    w->setEnabled(false);
                }
            },
        });

        // --- FcCompassDial stories ---
        FcGallery::StoryRegistry::instance().registerStories("FcCompassDial", {
            {
                "Default",
                "Compass dial at zero degrees",
                [](QWidget* w) {
                    setMetaProperty(w, "angle", 0.0);
                    w->setMinimumSize(120, 120);
                }
            },
            {
                "East",
                "Compass dial pointing east (90 degrees)",
                [](QWidget* w) {
                    setMetaProperty(w, "angle", 90.0);
                    w->setMinimumSize(120, 120);
                }
            },
            {
                "Southwest",
                "Compass dial at 225 degrees",
                [](QWidget* w) {
                    setMetaProperty(w, "angle", 225.0);
                    w->setMinimumSize(120, 120);
                }
            },
        });

        // --- FcPropertyListEditor stories ---
        FcGallery::StoryRegistry::instance().registerStories("FcPropertyListEditor", {
            {
                "Default",
                "Empty text editor with line numbers",
                [](QWidget* w) {
                    auto* editor = qobject_cast<QPlainTextEdit*>(w);
                    if (!editor) {
                        return;
                    }
                    editor->clear();
                    editor->setEnabled(true);
                    w->setMinimumSize(300, 150);
                }
            },
            {
                "With Content",
                "Editor with sample property list",
                [](QWidget* w) {
                    auto* editor = qobject_cast<QPlainTextEdit*>(w);
                    if (!editor) {
                        return;
                    }
                    editor->setPlainText(
                        "name = MyPart\n"
                        "length = 100.0\n"
                        "width = 50.0\n"
                        "height = 25.0\n"
                        "material = Steel\n"
                        "visible = true"
                    );
                    w->setMinimumSize(300, 150);
                }
            },
        });
    }
};

static DisplayStoriesRegistrar s_displayStories;

}  // namespace
