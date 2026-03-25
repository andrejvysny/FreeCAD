// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#pragma once

#include <QComboBox>
#include <QWidget>

namespace FcGallery
{

/// Compact theme switcher: System / Dark / Light combo box.
class ThemeSwitcher : public QWidget
{
    Q_OBJECT

public:
    explicit ThemeSwitcher(QWidget* parent = nullptr);

Q_SIGNALS:
    void themeChanged(const QString& name);

private Q_SLOTS:
    void onThemeSelected(int index);

private:
    void applyTheme(const QString& name);

    QComboBox* m_combo = nullptr;
};

}  // namespace FcGallery
