// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#include "ComponentBrowser.h"

#include <QFont>
#include <QHeaderView>

#include <FCComponentLib/Components/ComponentRegistry.h>

namespace FcGallery
{

ComponentBrowser::ComponentBrowser(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 8, 0, 8);
    layout->setSpacing(4);

    m_filter = new QLineEdit(this);
    m_filter->setPlaceholderText("Search components...");
    m_filter->setClearButtonEnabled(true);
    m_filter->setStyleSheet(
        "QLineEdit { border: 1px solid palette(mid); border-radius: 6px; "
        "padding: 6px 12px; margin: 0 8px 4px 8px; background: palette(base); }");
    layout->addWidget(m_filter);

    m_tree = new QTreeWidget(this);
    m_tree->setHeaderHidden(true);
    m_tree->setRootIsDecorated(false);
    m_tree->setIndentation(0);
    m_tree->setAnimated(true);
    m_tree->setFocusPolicy(Qt::NoFocus);
    m_tree->setStyleSheet(
        "QTreeWidget { border: none; background: transparent; outline: none; }"
        "QTreeWidget::item { padding: 6px 12px; border-radius: 6px; margin: 1px 8px; }"
        "QTreeWidget::item:selected "
        "{ background: palette(highlight); color: palette(highlighted-text); }"
        "QTreeWidget::item:hover:!selected { background: palette(alternate-base); }"
        "QTreeWidget::branch { image: none; width: 0px; }");
    layout->addWidget(m_tree);

    populate();

    connect(m_tree, &QTreeWidget::itemClicked, this, &ComponentBrowser::onItemClicked);
    connect(m_filter, &QLineEdit::textChanged, this, &ComponentBrowser::onFilterChanged);
}

void ComponentBrowser::populate()
{
    m_tree->clear();

    auto& registry = FcComponents::ComponentRegistry::instance();
    auto categories = registry.categories();

    for (const auto& category : categories) {
        auto* categoryItem = new QTreeWidgetItem(m_tree);
        auto components = registry.byCategory(category.c_str());
        categoryItem->setText(0, QString::fromStdString(category).toUpper());

        QFont catFont = categoryItem->font(0);
        catFont.setPointSize(9);
        catFont.setBold(true);
        categoryItem->setFont(0, catFont);
        categoryItem->setForeground(0, palette().color(QPalette::PlaceholderText));
        categoryItem->setFlags(categoryItem->flags() & ~Qt::ItemIsSelectable);

        for (const auto& info : components) {
            auto* item = new QTreeWidgetItem(categoryItem);
            item->setText(0, QString::fromUtf8(info.displayName));
            item->setData(0, Qt::UserRole, QString::fromUtf8(info.name));
            item->setToolTip(0, QString::fromUtf8(info.description));
        }

        categoryItem->setExpanded(true);
    }
}

void ComponentBrowser::onItemClicked(QTreeWidgetItem* item, int /*column*/)
{
    // Only leaf items (components) carry UserRole data
    QVariant data = item->data(0, Qt::UserRole);
    if (!data.isValid()) {
        return;
    }

    QString name = data.toString();
    auto& registry = FcComponents::ComponentRegistry::instance();
    for (const auto& info : registry.all()) {
        if (QString::fromUtf8(info.name) == name) {
            Q_EMIT componentSelected(info);
            return;
        }
    }
}

void ComponentBrowser::onFilterChanged(const QString& text)
{
    applyFilter(text);
}

void ComponentBrowser::applyFilter(const QString& filter)
{
    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        QTreeWidgetItem* category = m_tree->topLevelItem(i);
        bool anyCategoryMatch = false;

        for (int j = 0; j < category->childCount(); ++j) {
            QTreeWidgetItem* child = category->child(j);
            bool matches =
                filter.isEmpty() || child->text(0).contains(filter, Qt::CaseInsensitive);
            child->setHidden(!matches);
            if (matches) {
                anyCategoryMatch = true;
            }
        }

        category->setHidden(!anyCategoryMatch);
        if (anyCategoryMatch) {
            category->setExpanded(true);
        }
    }
}

}  // namespace FcGallery
