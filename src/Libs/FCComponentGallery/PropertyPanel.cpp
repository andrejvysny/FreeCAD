// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#include "PropertyPanel.h"

#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QMetaProperty>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include "ToggleSwitch.h"

namespace FcGallery
{

namespace
{

const char* headerStyle =
    "font-size: 11px; font-weight: 600; text-transform: uppercase; "
    "letter-spacing: 1px; padding: 8px 12px; "
    "color: palette(placeholder-text); background: transparent; "
    "border-bottom: 1px solid palette(midlight);";

}  // namespace

PropertyPanel::PropertyPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto* header = new QLabel("Properties", this);
    header->setStyleSheet(headerStyle);
    outerLayout->addWidget(header);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    outerLayout->addWidget(m_scrollArea);

    m_container = new QWidget(m_scrollArea);
    m_formLayout = new QFormLayout(m_container);
    m_formLayout->setContentsMargins(12, 8, 12, 8);
    m_formLayout->setSpacing(8);
    m_scrollArea->setWidget(m_container);
}

void PropertyPanel::setWidget(QWidget* widget, const QMetaObject* componentMeta)
{
    clearEditors();

    if (!widget) {
        return;
    }

    const QMetaObject* meta = widget->metaObject();

    // Skip inherited properties — only show the component's own custom properties.
    int offset = componentMeta ? componentMeta->superClass()->propertyCount()
                               : QWidget::staticMetaObject.propertyCount();

    for (int i = offset; i < meta->propertyCount(); ++i) {
        QMetaProperty prop = meta->property(i);
        if (!prop.isReadable()) {
            continue;
        }
        addPropertyEditor(widget, prop);
    }

    // Also show curated useful base-class properties
    static const char* alwaysShow[] = {"text", "enabled"};
    for (const char* name : alwaysShow) {
        int idx = meta->indexOfProperty(name);
        if (idx >= 0 && idx < offset) {
            QMetaProperty prop = meta->property(idx);
            if (prop.isReadable()) {
                addPropertyEditor(widget, prop);
            }
        }
    }

    // Empty state when no properties exist
    if (m_formLayout->count() == 0) {
        auto* emptyLabel = new QLabel("No properties", m_container);
        emptyLabel->setStyleSheet(
            "color: palette(placeholder-text); font-style: italic; padding: 8px;");
        m_formLayout->addRow(emptyLabel);
    }
}

void PropertyPanel::clearEditors()
{
    // Delete all rows from the form layout
    while (m_formLayout->count() > 0) {
        QLayoutItem* item = m_formLayout->takeAt(0);
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}

void PropertyPanel::addPropertyEditor(QWidget* target, const QMetaProperty& prop)
{
    QString label = QString::fromUtf8(prop.name());
    QVariant value = prop.read(target);
    int typeId = prop.typeId();

    if (typeId == QMetaType::Int) {
        auto* editor = new QSpinBox(m_container);
        editor->setRange(-999999, 999999);
        editor->setValue(value.toInt());
        connect(editor, &QSpinBox::valueChanged, target,
                [target, prop](int v) { prop.write(target, v); });
        m_formLayout->addRow(label, editor);
    }
    else if (typeId == QMetaType::Double) {
        auto* editor = new QDoubleSpinBox(m_container);
        editor->setRange(-999999.0, 999999.0);
        editor->setDecimals(4);
        editor->setValue(value.toDouble());
        connect(editor, &QDoubleSpinBox::valueChanged, target,
                [target, prop](double v) { prop.write(target, v); });
        m_formLayout->addRow(label, editor);
    }
    else if (typeId == QMetaType::Bool) {
        auto* editor = new ToggleSwitch(m_container);
        editor->setChecked(value.toBool());
        connect(editor, &QAbstractButton::toggled, target,
                [target, prop](bool v) { prop.write(target, v); });
        m_formLayout->addRow(label, editor);
    }
    else if (typeId == QMetaType::QString) {
        auto* editor = new QLineEdit(m_container);
        editor->setText(value.toString());
        connect(editor, &QLineEdit::textChanged, target,
                [target, prop](const QString& v) { prop.write(target, v); });
        m_formLayout->addRow(label, editor);
    }
    else if (value.canConvert<QColor>()) {
        auto* editor = new QPushButton(m_container);
        QColor color = value.value<QColor>();
        editor->setText(color.name(QColor::HexArgb));
        editor->setStyleSheet(
            QString("background-color: %1; border: 1px solid palette(mid); "
                    "border-radius: 4px; padding: 2px 8px;")
                .arg(color.name(QColor::HexArgb)));
        connect(editor, &QPushButton::clicked, target, [target, prop, editor]() {
            QColor current = prop.read(target).value<QColor>();
            QColor chosen = QColorDialog::getColor(current, editor, "Choose Color",
                                                   QColorDialog::ShowAlphaChannel);
            if (chosen.isValid()) {
                prop.write(target, chosen);
                editor->setText(chosen.name(QColor::HexArgb));
                editor->setStyleSheet(
                    QString("background-color: %1; border: 1px solid palette(mid); "
                            "border-radius: 4px; padding: 2px 8px;")
                        .arg(chosen.name(QColor::HexArgb)));
            }
        });
        m_formLayout->addRow(label, editor);
    }
    else {
        // Read-only label for unsupported types
        auto* editor = new QLabel(value.toString(), m_container);
        editor->setTextInteractionFlags(Qt::TextSelectableByMouse);
        m_formLayout->addRow(label, editor);
    }
}

}  // namespace FcGallery
