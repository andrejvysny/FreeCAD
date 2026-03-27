// SPDX-License-Identifier: LGPL-2.1-or-later

#include "WidgetBinder.h"
#include "AdapterRegistration.h"

#include <QObject>
#include <QWidget>

#include <exception>

using namespace Gui::Adapters;

namespace
{
const char* boundPropertyName()
{
    return "_fcGuiAdapterBound";
}
}

WidgetBinder& WidgetBinder::instance()
{
    static WidgetBinder binder;
    return binder;
}

bool WidgetBinder::attach(QWidget* widget)
{
    if (!widget) {
        return false;
    }

    if (widget->property(boundPropertyName()).toBool()) {
        return false;
    }

    RegisteredAdapter adapter;
    if (!AdapterRegistration::instance().findAdapter(widget->metaObject(), adapter)) {
        return false;
    }

    try {
        adapter.hooks.onAttach(*widget);
    }
    catch (const std::exception& e) {
        qWarning("Adapter attach failed for widget '%s': %s", widget->metaObject()->className(), e.what());
        return false;
    }
    catch (...) {
        qWarning("Adapter attach failed for widget '%s'", widget->metaObject()->className());
        return false;
    }

    widget->setProperty(boundPropertyName(), true);
    return true;
}

bool WidgetBinder::detach(QWidget* widget)
{
    if (!widget) {
        return false;
    }

    if (!widget->property(boundPropertyName()).toBool()) {
        return false;
    }

    RegisteredAdapter adapter;
    if (AdapterRegistration::instance().findAdapter(widget->metaObject(), adapter) && adapter.hooks.onDetach) {
        adapter.hooks.onDetach(*widget);
    }

    widget->setProperty(boundPropertyName(), false);
    return true;
}
