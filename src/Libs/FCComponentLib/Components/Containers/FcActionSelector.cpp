// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
 *   Copyright (c) 2025 FreeCAD Project Association                         *
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

#include "FcActionSelector.h"

#include <QApplication>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <FCComponentLib/Components/ComponentMeta.h>
#include <FCComponentLib/Components/ComponentRegistry.h>

namespace FcComponents
{

FcActionSelector::FcActionSelector(QWidget* parent)
    : QWidget(parent)
{
    m_addButton = new QPushButton(this);
    m_addButton->setObjectName(QLatin1String("addButton"));
    m_addButton->setMinimumSize(QSize(30, 30));
    m_addButton->setIcon(
        QIcon::fromTheme(QStringLiteral("go-next"),
                         style()->standardIcon(QStyle::SP_ArrowRight)));
    m_gridLayout = new QGridLayout(this);
    m_gridLayout->addWidget(m_addButton, 1, 1, 1, 1);

    m_spacerItem =
        new QSpacerItem(33, 57, QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_gridLayout->addItem(m_spacerItem, 5, 1, 1, 1);
    m_spacerItem1 =
        new QSpacerItem(33, 58, QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_gridLayout->addItem(m_spacerItem1, 0, 1, 1, 1);

    m_removeButton = new QPushButton(this);
    m_removeButton->setObjectName(QLatin1String("removeButton"));
    m_removeButton->setMinimumSize(QSize(30, 30));
    m_removeButton->setIcon(
        QIcon::fromTheme(QStringLiteral("go-previous"),
                         style()->standardIcon(QStyle::SP_ArrowLeft)));
    m_removeButton->setAutoDefault(true);
    m_removeButton->setDefault(false);

    m_gridLayout->addWidget(m_removeButton, 2, 1, 1, 1);

    m_upButton = new QPushButton(this);
    m_upButton->setObjectName(QLatin1String("upButton"));
    m_upButton->setMinimumSize(QSize(30, 30));
    m_upButton->setIcon(
        QIcon::fromTheme(QStringLiteral("go-up"),
                         style()->standardIcon(QStyle::SP_ArrowUp)));

    m_gridLayout->addWidget(m_upButton, 3, 1, 1, 1);

    m_downButton = new QPushButton(this);
    m_downButton->setObjectName(QLatin1String("downButton"));
    m_downButton->setMinimumSize(QSize(30, 30));
    m_downButton->setIcon(
        QIcon::fromTheme(QStringLiteral("go-down"),
                         style()->standardIcon(QStyle::SP_ArrowDown)));
    m_downButton->setAutoDefault(true);

    m_gridLayout->addWidget(m_downButton, 4, 1, 1, 1);

    m_vboxLayout = new QVBoxLayout();
    m_vboxLayout->setContentsMargins(0, 0, 0, 0);
    m_labelAvailable = new QLabel(this);
    m_vboxLayout->addWidget(m_labelAvailable);

    m_availableWidget = new QTreeWidget(this);
    m_availableWidget->setObjectName(QLatin1String("availableTreeWidget"));
    m_availableWidget->setRootIsDecorated(false);
    m_availableWidget->setHeaderLabels(QStringList() << QString());
    m_availableWidget->header()->hide();
    m_vboxLayout->addWidget(m_availableWidget);

    m_gridLayout->addLayout(m_vboxLayout, 0, 0, 6, 1);

    m_vboxLayout1 = new QVBoxLayout();
    m_vboxLayout1->setContentsMargins(0, 0, 0, 0);
    m_labelSelected = new QLabel(this);
    m_vboxLayout1->addWidget(m_labelSelected);

    m_selectedWidget = new QTreeWidget(this);
    m_selectedWidget->setObjectName(QLatin1String("selectedTreeWidget"));
    m_selectedWidget->setRootIsDecorated(false);
    m_selectedWidget->setHeaderLabels(QStringList() << QString());
    m_selectedWidget->header()->hide();
    m_vboxLayout1->addWidget(m_selectedWidget);

    m_gridLayout->addLayout(m_vboxLayout1, 0, 2, 6, 1);

    m_addButton->setText(QString());
    m_removeButton->setText(QString());
    m_upButton->setText(QString());
    m_downButton->setText(QString());

    connect(m_addButton, &QPushButton::clicked,
            this, &FcActionSelector::onAddButtonClicked);
    connect(m_removeButton, &QPushButton::clicked,
            this, &FcActionSelector::onRemoveButtonClicked);
    connect(m_upButton, &QPushButton::clicked,
            this, &FcActionSelector::onUpButtonClicked);
    connect(m_downButton, &QPushButton::clicked,
            this, &FcActionSelector::onDownButtonClicked);

    connect(m_availableWidget, &QTreeWidget::itemDoubleClicked,
            this, &FcActionSelector::onItemDoubleClicked);
    connect(m_availableWidget, &QTreeWidget::currentItemChanged,
            this, &FcActionSelector::onCurrentItemChanged);
    connect(m_selectedWidget, &QTreeWidget::itemDoubleClicked,
            this, &FcActionSelector::onItemDoubleClicked);
    connect(m_selectedWidget, &QTreeWidget::currentItemChanged,
            this, &FcActionSelector::onCurrentItemChanged);

    retranslateUi();
    setButtonsEnabled();
}

FcActionSelector::~FcActionSelector() = default;

QTreeWidget* FcActionSelector::availableTreeWidget() const
{
    return m_availableWidget;
}

QTreeWidget* FcActionSelector::selectedTreeWidget() const
{
    return m_selectedWidget;
}

void FcActionSelector::setSelectedLabel(const QString& label)
{
    m_labelSelected->setText(label);
}

QString FcActionSelector::selectedLabel() const
{
    return m_labelSelected->text();
}

void FcActionSelector::setAvailableLabel(const QString& label)
{
    m_labelAvailable->setText(label);
}

QString FcActionSelector::availableLabel() const
{
    return m_labelAvailable->text();
}

void FcActionSelector::retranslateUi()
{
    m_labelAvailable->setText(
        QApplication::translate("FcActionSelector", "Available:"));
    m_labelSelected->setText(
        QApplication::translate("FcActionSelector", "Selected:"));
    m_addButton->setToolTip(
        QApplication::translate("FcActionSelector", "Add"));
    m_removeButton->setToolTip(
        QApplication::translate("FcActionSelector", "Remove"));
    m_upButton->setToolTip(
        QApplication::translate("FcActionSelector", "Move up"));
    m_downButton->setToolTip(
        QApplication::translate("FcActionSelector", "Move down"));
}

void FcActionSelector::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

void FcActionSelector::keyPressEvent(QKeyEvent* event)
{
    if ((event->modifiers() & Qt::ControlModifier)) {
        switch (event->key()) {
            case Qt::Key_Right:
                onAddButtonClicked();
                break;
            case Qt::Key_Left:
                onRemoveButtonClicked();
                break;
            case Qt::Key_Up:
                onUpButtonClicked();
                break;
            case Qt::Key_Down:
                onDownButtonClicked();
                break;
            default:
                event->ignore();
                return;
        }
    }
}

void FcActionSelector::setButtonsEnabled()
{
    m_addButton->setEnabled(
        m_availableWidget->indexOfTopLevelItem(
            m_availableWidget->currentItem()) > -1);
    m_removeButton->setEnabled(
        m_selectedWidget->indexOfTopLevelItem(
            m_selectedWidget->currentItem()) > -1);
    m_upButton->setEnabled(
        m_selectedWidget->indexOfTopLevelItem(
            m_selectedWidget->currentItem()) > 0);
    m_downButton->setEnabled(
        m_selectedWidget->indexOfTopLevelItem(
            m_selectedWidget->currentItem()) > -1
        && m_selectedWidget->indexOfTopLevelItem(
               m_selectedWidget->currentItem())
            < m_selectedWidget->topLevelItemCount() - 1);
}

void FcActionSelector::onCurrentItemChanged(QTreeWidgetItem* /*current*/,
                                            QTreeWidgetItem* /*previous*/)
{
    setButtonsEnabled();
}

void FcActionSelector::onItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column);
    QTreeWidget* treeWidget = item->treeWidget();
    if (treeWidget == m_availableWidget) {
        int index = m_availableWidget->indexOfTopLevelItem(item);
        item = m_availableWidget->takeTopLevelItem(index);
        m_availableWidget->setCurrentItem(nullptr);
        m_selectedWidget->addTopLevelItem(item);
        m_selectedWidget->setCurrentItem(item);
    }
    else if (treeWidget == m_selectedWidget) {
        int index = m_selectedWidget->indexOfTopLevelItem(item);
        item = m_selectedWidget->takeTopLevelItem(index);
        m_selectedWidget->setCurrentItem(nullptr);
        m_availableWidget->addTopLevelItem(item);
        m_availableWidget->setCurrentItem(item);
    }
}

void FcActionSelector::onAddButtonClicked()
{
    QTreeWidgetItem* item = m_availableWidget->currentItem();
    if (item) {
        int index = m_availableWidget->indexOfTopLevelItem(item);
        item = m_availableWidget->takeTopLevelItem(index);
        m_availableWidget->setCurrentItem(nullptr);
        m_selectedWidget->addTopLevelItem(item);
        m_selectedWidget->setCurrentItem(item);
    }
}

void FcActionSelector::onRemoveButtonClicked()
{
    QTreeWidgetItem* item = m_selectedWidget->currentItem();
    if (item) {
        int index = m_selectedWidget->indexOfTopLevelItem(item);
        item = m_selectedWidget->takeTopLevelItem(index);
        m_selectedWidget->setCurrentItem(nullptr);
        m_availableWidget->addTopLevelItem(item);
        m_availableWidget->setCurrentItem(item);
    }
}

void FcActionSelector::onUpButtonClicked()
{
    QTreeWidgetItem* item = m_selectedWidget->currentItem();
    if (item && item->isSelected()) {
        int index = m_selectedWidget->indexOfTopLevelItem(item);
        if (index > 0) {
            m_selectedWidget->takeTopLevelItem(index);
            m_selectedWidget->insertTopLevelItem(index - 1, item);
            m_selectedWidget->setCurrentItem(item);
        }
    }
}

void FcActionSelector::onDownButtonClicked()
{
    QTreeWidgetItem* item = m_selectedWidget->currentItem();
    if (item && item->isSelected()) {
        int index = m_selectedWidget->indexOfTopLevelItem(item);
        if (index < m_selectedWidget->topLevelItemCount() - 1) {
            m_selectedWidget->takeTopLevelItem(index);
            m_selectedWidget->insertTopLevelItem(index + 1, item);
            m_selectedWidget->setCurrentItem(item);
        }
    }
}

}  // namespace FcComponents

FC_REGISTER_COMPONENT(FcActionSelector, "Containers", "Dual-panel item selector")
