// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#pragma once

#include <QPlainTextEdit>
#include <QPushButton>
#include <QWidget>

#include <FCComponentLib/Components/ComponentMeta.h>

namespace FcGallery
{

/// Bottom-right panel: read-only C++ code snippet with copy button.
class CodePreview : public QWidget
{
    Q_OBJECT

public:
    explicit CodePreview(QWidget* parent = nullptr);

    /// Generate and display a usage snippet for the given component.
    void setComponent(const FcComponents::ComponentInfo& info);

    /// Regenerate the snippet reflecting current widget property values.
    void refresh(const FcComponents::ComponentInfo& info, QWidget* widget,
                 const QMetaObject* componentMeta = nullptr);

private:
    QString generateSnippet(const FcComponents::ComponentInfo& info, QWidget* widget,
                            const QMetaObject* componentMeta = nullptr) const;

    QPlainTextEdit* m_editor = nullptr;
    QPushButton* m_copyBtn = nullptr;
};

}  // namespace FcGallery
