// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#pragma once

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

#include <FCComponentLib/Components/ComponentMeta.h>

namespace FcGallery
{

class ThemeSwitcher;

/// Center panel: renders a single live widget instance centered in a dot-grid frame.
class ComponentCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit ComponentCanvas(QWidget* parent = nullptr);

    /// Replace the displayed widget with a new instance from the given component.
    void setComponent(const FcComponents::ComponentInfo& info);

    /// Returns the currently displayed widget, or nullptr.
    QWidget* currentWidget() const;

    /// Set the theme switcher widget to display inline in the header bar.
    void setThemeSwitcher(QWidget* switcher);

private:
    QLabel* m_nameLabel = nullptr;
    QHBoxLayout* m_headerLayout = nullptr;
    QFrame* m_frame = nullptr;
    QVBoxLayout* m_frameLayout = nullptr;
    QWidget* m_currentWidget = nullptr;
};

}  // namespace FcGallery
