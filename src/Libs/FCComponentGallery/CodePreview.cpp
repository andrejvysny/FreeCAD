// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#include "CodePreview.h"

#include <QApplication>
#include <QClipboard>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QMetaProperty>
#include <QTimer>
#include <QVBoxLayout>

#include <memory>

namespace FcGallery
{

namespace
{

const char* headerStyle =
    "font-size: 11px; font-weight: 600; text-transform: uppercase; "
    "letter-spacing: 1px; padding: 8px 12px; "
    "color: palette(placeholder-text); background: transparent;";

const char* copyBtnStyle =
    "QPushButton { border: 1px solid palette(mid); border-radius: 4px; "
    "padding: 2px 10px; font-size: 10px; background: transparent; "
    "color: palette(highlight); }"
    "QPushButton:hover { background: palette(alternate-base); }"
    "QPushButton:pressed { background: palette(mid); }";

}  // namespace

CodePreview::CodePreview(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Header with copy button
    auto* headerWidget = new QWidget(this);
    headerWidget->setStyleSheet("border-bottom: 1px solid palette(midlight);");
    auto* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 8, 0);
    headerLayout->setSpacing(0);

    auto* headerLabel = new QLabel("Code", headerWidget);
    headerLabel->setStyleSheet(headerStyle);
    headerLayout->addWidget(headerLabel);

    headerLayout->addStretch();

    m_copyBtn = new QPushButton("Copy", headerWidget);
    m_copyBtn->setFixedHeight(22);
    m_copyBtn->setCursor(Qt::PointingHandCursor);
    m_copyBtn->setStyleSheet(copyBtnStyle);
    headerLayout->addWidget(m_copyBtn);

    layout->addWidget(headerWidget);

    connect(m_copyBtn, &QPushButton::clicked, this, [this]() {
        QApplication::clipboard()->setText(m_editor->toPlainText());
        m_copyBtn->setText("Copied!");
        QTimer::singleShot(1500, this, [this]() { m_copyBtn->setText("Copy"); });
    });

    // Code editor
    m_editor = new QPlainTextEdit(this);
    m_editor->setReadOnly(true);
    m_editor->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    m_editor->setStyleSheet("QPlainTextEdit { border: none; padding: 8px; }");

    QFont mono;
    mono.setFamilies({"Menlo", "Consolas", "Courier New", "monospace"});
    mono.setStyleHint(QFont::Monospace);
    mono.setPointSize(10);
    m_editor->setFont(mono);

    layout->addWidget(m_editor);
}

void CodePreview::setComponent(const FcComponents::ComponentInfo& info)
{
    m_editor->setPlainText(generateSnippet(info, nullptr));
}

void CodePreview::refresh(const FcComponents::ComponentInfo& info, QWidget* widget,
                          const QMetaObject* componentMeta)
{
    m_editor->setPlainText(generateSnippet(info, widget, componentMeta));
}

QString CodePreview::generateSnippet(const FcComponents::ComponentInfo& info, QWidget* widget,
                                     const QMetaObject* componentMeta) const
{
    QString name = QString::fromUtf8(info.name);
    QString category = QString::fromUtf8(info.category);

    QString code;
    code += QString("#include <FCComponentLib/Components/%1/%2.h>\n\n").arg(category, name);
    code += QString("auto* widget = new FcComponents::%1(parent);\n").arg(name);

    // Add setters for non-default property values
    if (widget) {
        const QMetaObject* meta = widget->metaObject();
        int offset = componentMeta ? componentMeta->superClass()->propertyCount()
                                   : QWidget::staticMetaObject.propertyCount();

        // Create a default instance to compare against
        std::unique_ptr<QWidget> defaultWidget(info.factory(nullptr));

        for (int i = offset; i < meta->propertyCount(); ++i) {
            QMetaProperty prop = meta->property(i);
            if (!prop.isReadable() || !prop.isWritable()) {
                continue;
            }

            QVariant value = prop.read(widget);

            // Skip properties that still equal their default value
            if (defaultWidget && prop.read(defaultWidget.get()) == value) {
                continue;
            }

            QString propName = QString::fromUtf8(prop.name());

            // Capitalize first letter for setter name
            QString setter = propName;
            if (!setter.isEmpty()) {
                setter[0] = setter[0].toUpper();
            }
            setter = "set" + setter;

            int typeId = prop.typeId();
            if (typeId == QMetaType::Int) {
                code += QString("widget->%1(%2);\n").arg(setter).arg(value.toInt());
            }
            else if (typeId == QMetaType::Double) {
                code += QString("widget->%1(%2);\n").arg(setter).arg(value.toDouble());
            }
            else if (typeId == QMetaType::Bool) {
                code +=
                    QString("widget->%1(%2);\n").arg(setter, value.toBool() ? "true" : "false");
            }
            else if (typeId == QMetaType::QString) {
                QString str = value.toString();
                if (!str.isEmpty()) {
                    code += QString("widget->%1(\"%2\");\n").arg(setter, str);
                }
            }
            else if (value.canConvert<QColor>()) {
                QColor c = value.value<QColor>();
                if (c.alpha() < 255) {
                    code += QString("widget->%1(QColor(%2, %3, %4, %5));\n")
                                .arg(setter)
                                .arg(c.red())
                                .arg(c.green())
                                .arg(c.blue())
                                .arg(c.alpha());
                }
                else {
                    code += QString("widget->%1(QColor(%2, %3, %4));\n")
                                .arg(setter)
                                .arg(c.red())
                                .arg(c.green())
                                .arg(c.blue());
                }
            }
        }
    }

    return code;
}

}  // namespace FcGallery
