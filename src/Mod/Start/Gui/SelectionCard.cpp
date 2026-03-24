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

#include <QFont>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QVBoxLayout>

#include "SelectionCard.h"
#include "FirstStartWidget.h"

using namespace StartGui;

SelectionCard::SelectionCard(const Config& config, QWidget* parent)
    : QFrame(parent)
{
    setObjectName(QStringLiteral("selectionCard"));
    setFrameShape(QFrame::NoFrame);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);
    updateStyle();

    _mainLayout = new QVBoxLayout(this);
    _mainLayout->setSpacing(6);
    _mainLayout->setContentsMargins(8, 8, 8, 8);

    // Badge (top-right, optional)
    if (!config.badge.isEmpty()) {
        _badgeLabel = new QLabel(config.badge, this);
        _badgeLabel->setObjectName(QStringLiteral("selectionCardBadge"));
        QFont badgeFont = _badgeLabel->font();
        badgeFont.setPointSize(9);
        badgeFont.setBold(true);
        _badgeLabel->setFont(badgeFont);
        _mainLayout->addWidget(_badgeLabel);
    }

    // Icon (optional)
    if (!config.icon.isNull()) {
        _iconLabel = new QLabel(this);
        _iconLabel->setPixmap(config.icon.pixmap(config.iconSize));
        _iconLabel->setAlignment(Qt::AlignCenter);
        _mainLayout->addWidget(_iconLabel);
    }

    // Title
    _titleLabel = new QLabel(config.title, this);
    _titleLabel->setObjectName(QStringLiteral("selectionCardTitle"));
    QFont titleFont = _titleLabel->font();
    titleFont.setBold(true);
    _titleLabel->setFont(titleFont);
    _titleLabel->setWordWrap(true);
    _mainLayout->addWidget(_titleLabel);

    // Description (optional)
    if (!config.description.isEmpty()) {
        _descriptionLabel = new QLabel(config.description, this);
        _descriptionLabel->setObjectName(QStringLiteral("selectionCardDescription"));
        _descriptionLabel->setWordWrap(true);
        _mainLayout->addWidget(_descriptionLabel);
    }

    updateStyle();
}

void SelectionCard::setChecked(bool checked)
{
    if (_checked == checked) {
        return;
    }
    _checked = checked;
    updateStyle();
    Q_EMIT toggled(_checked);
}

bool SelectionCard::isChecked() const
{
    return _checked;
}

void SelectionCard::setTitle(const QString& title)
{
    if (_titleLabel) {
        _titleLabel->setText(title);
    }
}

void SelectionCard::setDescription(const QString& description)
{
    if (_descriptionLabel) {
        _descriptionLabel->setText(description);
    }
    else if (!description.isEmpty()) {
        _descriptionLabel = new QLabel(description, this);
        _descriptionLabel->setObjectName(QStringLiteral("selectionCardDescription"));
        _descriptionLabel->setWordWrap(true);
        _mainLayout->addWidget(_descriptionLabel);
    }
}

void SelectionCard::setBadge(const QString& badge)
{
    if (_badgeLabel) {
        _badgeLabel->setText(badge);
        _badgeLabel->setVisible(!badge.isEmpty());
    }
}

void SelectionCard::setContentWidget(QWidget* widget)
{
    if (_contentWidget) {
        _mainLayout->removeWidget(_contentWidget);
        _contentWidget->deleteLater();
    }
    _contentWidget = widget;
    if (widget) {
        // Insert content after icon, before title
        int insertIndex = _iconLabel ? _mainLayout->indexOf(_iconLabel) + 1 : 0;
        _mainLayout->insertWidget(insertIndex, widget);
    }
}

void SelectionCard::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        setChecked(true);
        Q_EMIT clicked();
    }
    QFrame::mousePressEvent(event);
}

void SelectionCard::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return) {
        setChecked(true);
        Q_EMIT clicked();
        return;
    }
    QFrame::keyPressEvent(event);
}

void SelectionCard::updateStyle()
{
    bool dark = isWizardDarkMode();
    QString bg = dark ? QStringLiteral("#383838") : QStringLiteral("white");
    QString border = _checked ? QStringLiteral("#418FDE")
                              : (dark ? QStringLiteral("#555") : QStringLiteral("#d0d0d0"));

    QString css = QString(
        "QFrame#selectionCard {"
        "  border: 2px solid %1;"
        "  border-radius: 8px;"
        "  padding: 8px;"
        "  background-color: %2;"
        "}"
    ).arg(border, bg);

    if (!_checked) {
        css += QStringLiteral(
            "QFrame#selectionCard:hover {"
            "  border-color: #418FDE;"
            "}"
        );
    }

    setStyleSheet(css);
}
