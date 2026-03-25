// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QKeySequenceEdit>
#include <QLineEdit>
#include <QSpinBox>

#include "StoryRegistry.h"

namespace
{

struct InputStoriesRegistrar
{
    InputStoriesRegistrar()
    {
        // --- FcSpinBox stories ---
        FcGallery::StoryRegistry::instance().registerStories("FcSpinBox", {
            {
                "Default",
                "Spin box with default range",
                [](QWidget* w) {
                    auto* sb = qobject_cast<QSpinBox*>(w);
                    if (!sb) {
                        return;
                    }
                    sb->setRange(0, 99);
                    sb->setValue(0);
                    sb->setEnabled(true);
                }
            },
            {
                "Large Range",
                "Spin box with range 0-10000",
                [](QWidget* w) {
                    auto* sb = qobject_cast<QSpinBox*>(w);
                    if (!sb) {
                        return;
                    }
                    sb->setRange(0, 10000);
                    sb->setValue(5000);
                    sb->setSingleStep(100);
                }
            },
            {
                "Disabled",
                "Spin box with interaction disabled",
                [](QWidget* w) {
                    auto* sb = qobject_cast<QSpinBox*>(w);
                    if (!sb) {
                        return;
                    }
                    sb->setRange(0, 99);
                    sb->setValue(42);
                    sb->setEnabled(false);
                }
            },
            {
                "No Scroll Guard",
                "Spin box that does not block wheel events",
                [](QWidget* w) {
                    auto* sb = qobject_cast<QSpinBox*>(w);
                    if (!sb) {
                        return;
                    }
                    sb->setRange(0, 99);
                    sb->setValue(0);
                    // FcSpinBox normally installs a scroll guard;
                    // this story tests without it by setting focus policy.
                    sb->setFocusPolicy(Qt::StrongFocus);
                }
            },
        });

        // --- FcDoubleSpinBox stories ---
        FcGallery::StoryRegistry::instance().registerStories("FcDoubleSpinBox", {
            {
                "Default",
                "Double spin box with default precision",
                [](QWidget* w) {
                    auto* sb = qobject_cast<QDoubleSpinBox*>(w);
                    if (!sb) {
                        return;
                    }
                    sb->setRange(0.0, 100.0);
                    sb->setValue(0.0);
                    sb->setDecimals(2);
                }
            },
            {
                "High Precision",
                "Double spin box with 3 decimal places",
                [](QWidget* w) {
                    auto* sb = qobject_cast<QDoubleSpinBox*>(w);
                    if (!sb) {
                        return;
                    }
                    sb->setRange(0.0, 100.0);
                    sb->setValue(3.141);
                    sb->setDecimals(3);
                    sb->setSingleStep(0.001);
                }
            },
        });

        // --- FcLineEdit stories ---
        FcGallery::StoryRegistry::instance().registerStories("FcLineEdit", {
            {
                "Default",
                "Line edit in its default state",
                [](QWidget* w) {
                    auto* le = qobject_cast<QLineEdit*>(w);
                    if (!le) {
                        return;
                    }
                    le->clear();
                    le->setPlaceholderText({});
                    le->setEchoMode(QLineEdit::Normal);
                    le->setEnabled(true);
                }
            },
            {
                "With Hint",
                "Line edit with placeholder text",
                [](QWidget* w) {
                    auto* le = qobject_cast<QLineEdit*>(w);
                    if (!le) {
                        return;
                    }
                    le->clear();
                    le->setPlaceholderText("Enter text...");
                    le->setEchoMode(QLineEdit::Normal);
                }
            },
            {
                "Password",
                "Line edit with password echo mode",
                [](QWidget* w) {
                    auto* le = qobject_cast<QLineEdit*>(w);
                    if (!le) {
                        return;
                    }
                    le->setPlaceholderText("Password");
                    le->setEchoMode(QLineEdit::Password);
                    le->setText("secret");
                }
            },
        });

        // --- FcComboBox stories ---
        FcGallery::StoryRegistry::instance().registerStories("FcComboBox", {
            {
                "Default",
                "Combo box with sample items",
                [](QWidget* w) {
                    auto* cb = qobject_cast<QComboBox*>(w);
                    if (!cb) {
                        return;
                    }
                    cb->clear();
                    cb->addItems({"Option A", "Option B", "Option C"});
                    cb->setEditable(false);
                    cb->setEnabled(true);
                }
            },
            {
                "Editable",
                "Combo box allowing custom user input",
                [](QWidget* w) {
                    auto* cb = qobject_cast<QComboBox*>(w);
                    if (!cb) {
                        return;
                    }
                    cb->clear();
                    cb->addItems({"Preset 1", "Preset 2", "Preset 3"});
                    cb->setEditable(true);
                    cb->setCurrentText("Custom value");
                }
            },
        });
    }
};

static InputStoriesRegistrar s_inputStories;


struct ExtraInputStoriesRegistrar
{
    ExtraInputStoriesRegistrar()
    {
        // --- FcElideCheckBox stories ---
        FcGallery::StoryRegistry::instance().registerStories("FcElideCheckBox", {
            {
                "Default",
                "Checkbox with short text",
                [](QWidget* w) {
                    auto* cb = qobject_cast<QCheckBox*>(w);
                    if (!cb) {
                        return;
                    }
                    cb->setText("Enable feature");
                    cb->setChecked(false);
                    cb->setEnabled(true);
                }
            },
            {
                "Long Text",
                "Checkbox with long text that gets elided",
                [](QWidget* w) {
                    auto* cb = qobject_cast<QCheckBox*>(w);
                    if (!cb) {
                        return;
                    }
                    cb->setText("This is a very long label that should be elided "
                                "when there is not enough horizontal space");
                    cb->setChecked(false);
                    w->setMaximumWidth(200);
                }
            },
            {
                "Checked",
                "Checkbox in checked state",
                [](QWidget* w) {
                    auto* cb = qobject_cast<QCheckBox*>(w);
                    if (!cb) {
                        return;
                    }
                    cb->setText("Checked option");
                    cb->setChecked(true);
                }
            },
        });

        // --- FcAccelLineEdit stories ---
        FcGallery::StoryRegistry::instance().registerStories("FcAccelLineEdit", {
            {
                "Default",
                "Empty shortcut input",
                [](QWidget* w) {
                    auto* kse = qobject_cast<QKeySequenceEdit*>(w);
                    if (!kse) {
                        return;
                    }
                    kse->clear();
                    kse->setEnabled(true);
                }
            },
            {
                "With Shortcut",
                "Input showing Ctrl+S shortcut",
                [](QWidget* w) {
                    auto* kse = qobject_cast<QKeySequenceEdit*>(w);
                    if (!kse) {
                        return;
                    }
                    kse->setKeySequence(QKeySequence(Qt::CTRL | Qt::Key_S));
                }
            },
            {
                "Disabled",
                "Shortcut input with interaction disabled",
                [](QWidget* w) {
                    w->setEnabled(false);
                }
            },
        });

        // --- FcClearLineEdit stories ---
        FcGallery::StoryRegistry::instance().registerStories("FcClearLineEdit", {
            {
                "Default",
                "Empty line edit (clear button hidden)",
                [](QWidget* w) {
                    auto* le = qobject_cast<QLineEdit*>(w);
                    if (!le) {
                        return;
                    }
                    le->clear();
                    le->setPlaceholderText("Type to see clear button...");
                    le->setEnabled(true);
                }
            },
            {
                "With Text",
                "Line edit with text showing clear button",
                [](QWidget* w) {
                    auto* le = qobject_cast<QLineEdit*>(w);
                    if (!le) {
                        return;
                    }
                    le->setText("Some searchable text");
                }
            },
        });

        // --- FcModifierLineEdit stories ---
        FcGallery::StoryRegistry::instance().registerStories("FcModifierLineEdit", {
            {
                "Default",
                "Empty modifier input",
                [](QWidget* w) {
                    auto* le = qobject_cast<QLineEdit*>(w);
                    if (!le) {
                        return;
                    }
                    le->clear();
                    le->setPlaceholderText("Press modifier keys...");
                    le->setEnabled(true);
                }
            },
        });

        // --- FcImageTextEdit stories ---
        FcGallery::StoryRegistry::instance().registerStories("FcImageTextEdit", {
            {
                "Default",
                "Empty text editor with image drop support",
                [](QWidget* w) {
                    w->setEnabled(true);
                    w->setMinimumSize(300, 100);
                }
            },
        });
    }
};

static ExtraInputStoriesRegistrar s_extraInputStories;

}  // namespace
