// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
 *   Copyright (c) 2025 The FreeCAD Project Association AISBL               *
 *                                                                          *
 *   This file is part of FreeCAD.                                          *
 *                                                                          *
 *   FreeCAD is free software: you can redistribute it and/or modify it     *
 *   under the terms of the GNU Lesser General Public License as            *
 *   published by the Free Software Foundation, either version 2.1 of the   *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   FreeCAD is distributed in the hope that it will be useful, but         *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with FreeCAD. If not, see                                *
 *   <https://www.gnu.org/licenses/>.                                       *
 *                                                                          *
 ***************************************************************************/

#pragma once

#include <QFrame>
#include <QIcon>
#include <QString>

class QLabel;
class QVBoxLayout;

namespace StartGui
{

/// Reusable selectable card widget for wizard option selection.
/// Displays an icon, title, optional description, and optional badge.
/// Supports checkable state with visual border highlight.
class SelectionCard: public QFrame
{
    Q_OBJECT
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked NOTIFY toggled)

public:
    struct Config
    {
        QString title;
        QString description;
        QIcon icon;
        QSize iconSize = {48, 48};
        QString badge;
    };

    explicit SelectionCard(const Config& config, QWidget* parent = nullptr);

    void setChecked(bool checked);
    bool isChecked() const;

    void setTitle(const QString& title);
    void setDescription(const QString& description);
    void setBadge(const QString& badge);

    /// Set a custom content widget below the icon (e.g., for nav card text mappings)
    void setContentWidget(QWidget* widget);

    /// Re-apply card styles (called on theme change)
    void updateStyle();

Q_SIGNALS:
    void clicked();
    void toggled(bool checked);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:

    bool _checked = false;
    QLabel* _iconLabel = nullptr;
    QLabel* _titleLabel = nullptr;
    QLabel* _descriptionLabel = nullptr;
    QLabel* _badgeLabel = nullptr;
    QVBoxLayout* _mainLayout = nullptr;
    QWidget* _contentWidget = nullptr;
};

}  // namespace StartGui
