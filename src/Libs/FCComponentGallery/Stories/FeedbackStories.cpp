// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#include <QMetaProperty>
#include <QPushButton>

#include "StoryRegistry.h"

// FcColorButton is a QPushButton subclass with color-specific Q_PROPERTYs.
// We cast to QPushButton for safe access and use QMetaProperty for custom props.

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

struct FeedbackStoriesRegistrar
{
    FeedbackStoriesRegistrar()
    {
        FcGallery::StoryRegistry::instance().registerStories("FcColorButton", {
            {
                "Default",
                "Color button showing red",
                [](QWidget* w) {
                    w->setEnabled(true);
                    setMetaProperty(w, "color", QColor(Qt::red));
                    setMetaProperty(w, "allowTransparency", false);
                }
            },
            {
                "With Transparency",
                "Color button with transparency enabled, semi-transparent blue",
                [](QWidget* w) {
                    w->setEnabled(true);
                    setMetaProperty(w, "allowTransparency", true);
                    setMetaProperty(w, "color", QColor(0, 100, 255, 128));
                }
            },
            {
                "Disabled",
                "Color button with interaction disabled",
                [](QWidget* w) {
                    setMetaProperty(w, "color", QColor(Qt::gray));
                    w->setEnabled(false);
                }
            },
        });
    }
};

static FeedbackStoriesRegistrar s_feedbackStories;

}  // namespace
