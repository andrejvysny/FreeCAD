// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#pragma once

#include <QFormLayout>
#include <QScrollArea>
#include <QWidget>

namespace FcGallery
{

/// Right panel: QMetaObject-driven Q_PROPERTY editor for the live widget.
///
/// Enumerates Q_PROPERTYs starting from the component's own property offset
/// (skipping base QWidget properties) and creates appropriate editors:
/// int->QSpinBox, double->QDoubleSpinBox, bool->QCheckBox, QString->QLineEdit,
/// QColor->QPushButton with QColorDialog, others->read-only QLabel.
class PropertyPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PropertyPanel(QWidget* parent = nullptr);

    /// Rebuild the property editor for the given widget.
    void setWidget(QWidget* widget, const QMetaObject* componentMeta = nullptr);

private:
    void clearEditors();
    void addPropertyEditor(QWidget* target, const QMetaProperty& prop);

    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_container = nullptr;
    QFormLayout* m_formLayout = nullptr;
};

}  // namespace FcGallery
