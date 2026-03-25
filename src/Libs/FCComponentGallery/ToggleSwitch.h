// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#pragma once

#include <QAbstractButton>

namespace FcGallery
{

/// Animated toggle switch for boolean properties. Replaces QCheckBox with a modern look.
class ToggleSwitch : public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY(qreal thumbPosition READ thumbPosition WRITE setThumbPosition)

public:
    explicit ToggleSwitch(QWidget* parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    qreal thumbPosition() const;
    void setThumbPosition(qreal pos);

protected:
    void paintEvent(QPaintEvent* event) override;
    void checkStateSet() override;
    void nextCheckState() override;

private:
    void startAnimation();

    qreal m_thumbPosition = 0.0;
};

}  // namespace FcGallery
