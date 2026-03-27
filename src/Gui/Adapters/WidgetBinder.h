// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

class QWidget;

namespace Gui
{
namespace Adapters
{

class WidgetBinder
{
public:
    static WidgetBinder& instance();

    bool attach(QWidget* widget);
    bool detach(QWidget* widget);

private:
    WidgetBinder() = default;
};

}
}
