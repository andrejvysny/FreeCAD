// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#pragma once

#include <QLineEdit>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>

#include <FCComponentLib/Components/ComponentMeta.h>

namespace FcGallery
{

/// Left sidebar: tree of components grouped by category, with search filter.
class ComponentBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit ComponentBrowser(QWidget* parent = nullptr);

Q_SIGNALS:
    void componentSelected(const FcComponents::ComponentInfo& info);

private Q_SLOTS:
    void onItemClicked(QTreeWidgetItem* item, int column);
    void onFilterChanged(const QString& text);

private:
    void populate();
    void applyFilter(const QString& filter);

    QLineEdit* m_filter = nullptr;
    QTreeWidget* m_tree = nullptr;
};

}  // namespace FcGallery
