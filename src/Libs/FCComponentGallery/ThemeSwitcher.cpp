// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#include "ThemeSwitcher.h"

#include <QApplication>
#include <QHBoxLayout>

#include <FCComponentLib/Style/ComponentPalette.h>

namespace FcGallery
{

ThemeSwitcher::ThemeSwitcher(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_combo = new QComboBox(this);
    m_combo->addItems({"System", "Dark", "Light"});
    m_combo->setMinimumWidth(80);
    m_combo->setStyleSheet(
        "QComboBox { border: 1px solid palette(mid); border-radius: 4px; "
        "padding: 2px 8px; }");
    layout->addWidget(m_combo);

    connect(m_combo, &QComboBox::currentIndexChanged, this, &ThemeSwitcher::onThemeSelected);

    // Default to System
    m_combo->setCurrentIndex(0);
}

void ThemeSwitcher::onThemeSelected(int /*index*/)
{
    QString name = m_combo->currentText();
    applyTheme(name);
    Q_EMIT themeChanged(name);
}

void ThemeSwitcher::applyTheme(const QString& name)
{
    static const QString paletteQss = QStringLiteral(
        "QWidget { background-color: palette(window); color: palette(window-text); }"
        "QTreeWidget, QTreeView, QListWidget, QListView, QTableView, QPlainTextEdit, "
        "QTextEdit { background-color: palette(base); color: palette(text); }"
        "QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox "
        "{ background-color: palette(base); color: palette(text); }"
        "QToolBar { background-color: palette(window); border: none; }"
        "QScrollArea { border: none; background: transparent; }");

    if (name == "Dark") {
        QApplication::setPalette(FcComponents::ComponentPalette::darkPalette());
        qApp->setStyleSheet(paletteQss);
    }
    else if (name == "Light") {
        QApplication::setPalette(FcComponents::ComponentPalette::lightPalette());
        qApp->setStyleSheet(paletteQss);
    }
    else {
        // "System" — reset to native
        QApplication::setPalette(QApplication::style()->standardPalette());
        qApp->setStyleSheet(QString());
    }
}

}  // namespace FcGallery
