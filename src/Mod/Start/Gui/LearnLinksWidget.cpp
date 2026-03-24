// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
 *   Copyright (c) 2024 The FreeCAD Project Association AISBL               *
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

#include <QCursor>
#include <QDesktopServices>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

#include <App/Application.h>

#include "LearnLinksWidget.h"

using namespace StartGui;

namespace
{

QPushButton* makeLearnLink(const QString& text, const QString& iconPath, const QString& url)
{
    auto* button = new QPushButton(QIcon(iconPath), text);
    button->setObjectName(QStringLiteral("learnLink"));
    button->setFlat(true);
    button->setCursor(Qt::PointingHandCursor);
    QObject::connect(button, &QPushButton::clicked, [url]() {
        QDesktopServices::openUrl(QUrl(url));
    });
    return button;
}

}  // namespace

QString LearnLinksWidget::releaseNotesUrl()
{
    QString version = QString::fromStdString(App::Application::Config()["ExeVersion"]);
    QStringList parts = version.split(QLatin1Char('.'));
    QString majorMinor =
        parts.size() >= 2 ? parts.at(0) + QLatin1Char('_') + parts.at(1) : version;
    return QStringLiteral("https://wiki.freecad.org/Release_notes_") + majorMinor;
}

QString LearnLinksWidget::whatsNewLabel()
{
    QString version = QString::fromStdString(App::Application::Config()["ExeVersion"]);
    QStringList parts = version.split(QLatin1Char('.'));
    QString majorMinor =
        parts.size() >= 2 ? parts.at(0) + QLatin1Char('.') + parts.at(1) : version;
    return majorMinor;
}

LearnLinksWidget::LearnLinksWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("learnSection"));

    auto* layout = new QVBoxLayout(this);

    layout->addWidget(makeLearnLink(
        tr("Beginner tutorial"),
        QStringLiteral(":/icons/help-browser.svg"),
        QStringLiteral("https://wiki.freecad.org/Getting_started")
    ));
    layout->addWidget(makeLearnLink(
        tr("Documentation"),
        QStringLiteral(":/icons/document-open.svg"),
        QStringLiteral("https://wiki.freecad.org/User_hub")
    ));
    layout->addWidget(makeLearnLink(
        tr("What's new in %1").arg(whatsNewLabel()),
        QStringLiteral(":/icons/internet-web-browser.svg"),
        releaseNotesUrl()
    ));
    layout->addWidget(makeLearnLink(
        tr("Forum"),
        QStringLiteral(":/icons/internet-web-browser.svg"),
        QStringLiteral("https://forum.freecad.org")
    ));
    layout->addWidget(makeLearnLink(
        tr("Community"),
        QStringLiteral(":/icons/internet-web-browser.svg"),
        QStringLiteral("https://freecad.org/community")
    ));
}
