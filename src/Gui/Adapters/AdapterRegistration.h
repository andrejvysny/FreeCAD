// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <functional>
#include <map>
#include <string>

class QMetaObject;
class QWidget;

namespace Gui
{
namespace Adapters
{

using AdapterAttachHook = std::function<void(QWidget&)>;
using AdapterDetachHook = std::function<void(QWidget&)>;

struct AdapterHooks
{
    AdapterAttachHook onAttach;
    AdapterDetachHook onDetach;
};

struct RegisteredAdapter
{
    std::string widgetType;
    AdapterHooks hooks;
};

class AdapterRegistration
{
public:
    static AdapterRegistration& instance();

    bool registerAdapter(const std::string& widgetType, AdapterHooks hooks);
    bool unregisterAdapter(const std::string& widgetType);
    void clear();

    bool findAdapter(const QMetaObject* metaObject, RegisteredAdapter& outAdapter) const;

private:
    AdapterRegistration() = default;

    std::map<std::string, AdapterHooks, std::less<std::string>> registrations;
};

}
}
