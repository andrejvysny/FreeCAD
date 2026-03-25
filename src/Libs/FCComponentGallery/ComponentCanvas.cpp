// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2025 FreeCAD Project Association

#include "ComponentCanvas.h"

#include <QLabel>
#include <QPainter>

namespace FcGallery
{

namespace
{

/// Frame that paints a subtle dot grid behind the component preview.
class DotGridFrame : public QFrame
{
protected:
    void paintEvent(QPaintEvent* event) override
    {
        QFrame::paintEvent(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QColor dotColor = palette().color(QPalette::Mid);
        dotColor.setAlpha(80);
        painter.setPen(Qt::NoPen);
        painter.setBrush(dotColor);

        constexpr int spacing = 20;
        constexpr qreal radius = 1.0;

        for (int x = spacing; x < width(); x += spacing) {
            for (int y = spacing; y < height(); y += spacing) {
                painter.drawEllipse(QPointF(x, y), radius, radius);
            }
        }
    }
};

}  // namespace

ComponentCanvas::ComponentCanvas(QWidget* parent)
    : QWidget(parent)
{
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(8, 8, 8, 8);
    outerLayout->setSpacing(8);

    // Header bar: component name + theme switcher (fixed height)
    auto* headerWidget = new QWidget(this);
    headerWidget->setMaximumHeight(32);
    headerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_headerLayout = new QHBoxLayout(headerWidget);
    m_headerLayout->setContentsMargins(0, 0, 0, 0);

    m_nameLabel = new QLabel(headerWidget);
    m_nameLabel->setStyleSheet(
        "font-size: 14px; font-weight: bold; padding: 2px 0px; color: palette(window-text);");
    m_nameLabel->setVisible(false);
    m_headerLayout->addWidget(m_nameLabel);
    m_headerLayout->addStretch();

    outerLayout->addWidget(headerWidget);

    // Dot-grid preview frame
    m_frame = new DotGridFrame();
    m_frame->setFrameStyle(QFrame::NoFrame);
    m_frame->setStyleSheet(
        "QFrame { background: palette(base); border: 1px solid palette(midlight); "
        "border-radius: 8px; }");
    outerLayout->addWidget(m_frame);

    m_frameLayout = new QVBoxLayout(m_frame);
    m_frameLayout->setContentsMargins(20, 20, 20, 20);
    m_frameLayout->setAlignment(Qt::AlignCenter);

    // Placeholder label
    auto* placeholder = new QLabel("Select a component from the browser", m_frame);
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setStyleSheet("color: palette(placeholder-text); font-style: italic;");
    m_frameLayout->addWidget(placeholder);
}

void ComponentCanvas::setComponent(const FcComponents::ComponentInfo& info)
{
    // Remove old widget
    if (m_currentWidget) {
        m_frameLayout->removeWidget(m_currentWidget);
        delete m_currentWidget;
        m_currentWidget = nullptr;
    }

    // Remove placeholder if present
    while (m_frameLayout->count() > 0) {
        QLayoutItem* item = m_frameLayout->takeAt(0);
        delete item->widget();
        delete item;
    }

    // Update name label
    m_nameLabel->setText(QString("%1 — %2")
                             .arg(QString::fromUtf8(info.displayName),
                                  QString::fromUtf8(info.description)));
    m_nameLabel->setVisible(true);

    // Create new widget via factory
    m_currentWidget = info.factory(m_frame);
    if (m_currentWidget) {
        m_frameLayout->addWidget(m_currentWidget);
    }
}

QWidget* ComponentCanvas::currentWidget() const
{
    return m_currentWidget;
}

void ComponentCanvas::setThemeSwitcher(QWidget* switcher)
{
    if (switcher) {
        m_headerLayout->addWidget(switcher);
    }
}

}  // namespace FcGallery
