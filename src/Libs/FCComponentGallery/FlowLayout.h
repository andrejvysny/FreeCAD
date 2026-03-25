// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2024 Werner Mayer <wmayer[at]users.sourceforge.net>
// Copyright (c) 2025 FreeCAD Project Association

#pragma once

#include <QLayout>
#include <QList>
#include <QStyle>

namespace FcGallery
{

/// Flow layout that wraps child widgets to the next row when horizontal space runs out.
/// Copied from src/Mod/Start/Gui/FlowLayout with namespace change.
class FlowLayout : public QLayout
{
    Q_OBJECT

public:
    explicit FlowLayout(QWidget* parent = nullptr, int margin = -1, int hSpacing = -1,
                         int vSpacing = -1);
    ~FlowLayout() override;

    void addItem(QLayoutItem* item) override;
    int count() const override;
    QLayoutItem* itemAt(int index) const override;
    QLayoutItem* takeAt(int index) override;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;
    void setGeometry(const QRect& rect) override;
    QSize sizeHint() const override;
    QSize minimumSize() const override;

private:
    int horizontalSpacing() const;
    int verticalSpacing() const;
    int smartSpacing(QStyle::PixelMetric pm) const;
    int doLayout(const QRect& rect, bool testOnly) const;

private:
    QList<QLayoutItem*> itemList;
    int hSpace = -1;
    int vSpace = -1;
};

}  // namespace FcGallery
