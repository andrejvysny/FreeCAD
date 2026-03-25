// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#include "ToggleSwitch.h"

#include <QPainter>
#include <QPropertyAnimation>

namespace FcGallery
{

ToggleSwitch::ToggleSwitch(QWidget* parent)
    : QAbstractButton(parent)
{
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setFixedSize(40, 22);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

QSize ToggleSwitch::sizeHint() const
{
    return {40, 22};
}

QSize ToggleSwitch::minimumSizeHint() const
{
    return sizeHint();
}

qreal ToggleSwitch::thumbPosition() const
{
    return m_thumbPosition;
}

void ToggleSwitch::setThumbPosition(qreal pos)
{
    m_thumbPosition = pos;
    update();
}

void ToggleSwitch::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int w = width();
    const int h = height();
    const int trackH = 16;
    const int trackY = (h - trackH) / 2;
    const qreal trackR = trackH / 2.0;

    // Track
    QRectF trackRect(0, trackY, w, trackH);
    QColor trackColor = isChecked() ? palette().color(QPalette::Highlight)
                                    : palette().color(QPalette::Mid);
    p.setPen(Qt::NoPen);
    p.setBrush(trackColor);
    p.drawRoundedRect(trackRect, trackR, trackR);

    // Thumb
    const int thumbD = 18;
    const qreal thumbR = thumbD / 2.0;
    const qreal minX = thumbR + 1;
    const qreal maxX = w - thumbR - 1;
    const qreal cx = minX + m_thumbPosition * (maxX - minX);
    const qreal cy = h / 2.0;

    p.setBrush(QColor(0xFF, 0xFF, 0xFF));
    p.setPen(Qt::NoPen);
    p.drawEllipse(QPointF(cx, cy), thumbR, thumbR);
}

void ToggleSwitch::checkStateSet()
{
    QAbstractButton::checkStateSet();
    startAnimation();
}

void ToggleSwitch::nextCheckState()
{
    QAbstractButton::nextCheckState();
    startAnimation();
}

void ToggleSwitch::startAnimation()
{
    auto* anim = new QPropertyAnimation(this, "thumbPosition", this);
    anim->setDuration(150);
    anim->setStartValue(m_thumbPosition);
    anim->setEndValue(isChecked() ? 1.0 : 0.0);
    anim->setEasingCurve(QEasingCurve::InOutQuad);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

}  // namespace FcGallery

#include "moc_ToggleSwitch.cpp"
