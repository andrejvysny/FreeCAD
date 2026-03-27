// SPDX-License-Identifier: LGPL-2.1-or-later

#include "AdapterRegistration.h"

#include <QObject>
#include <utility>

using namespace Gui::Adapters;

AdapterRegistration& AdapterRegistration::instance()
{
    static AdapterRegistration registration;
    return registration;
}

bool AdapterRegistration::registerAdapter(const std::string& widgetType, AdapterHooks hooks)
{
    if (widgetType.empty() || !hooks.onAttach) {
        return false;
    }

    registrations[widgetType] = std::move(hooks);
    return true;
}

bool AdapterRegistration::unregisterAdapter(const std::string& widgetType)
{
    return registrations.erase(widgetType) > 0;
}

void AdapterRegistration::clear()
{
    registrations.clear();
}

bool AdapterRegistration::findAdapter(const QMetaObject* metaObject, RegisteredAdapter& outAdapter) const
{
    for (auto current = metaObject; current; current = current->superClass()) {
        auto it = registrations.find(current->className());
        if (it != registrations.end()) {
            outAdapter.widgetType = it->first;
            outAdapter.hooks = it->second;
            return true;
        }
    }

    return false;
}
