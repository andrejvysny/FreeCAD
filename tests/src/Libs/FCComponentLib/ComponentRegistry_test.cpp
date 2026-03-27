// SPDX-License-Identifier: LGPL-2.1-or-later

#include <gtest/gtest.h>

#include <QApplication>
#include <QCoreApplication>
#include <QWidget>

#include <FCComponentLib/Components/ComponentRegistry.h>

namespace
{

QApplication* ensureApplication()
{
    if (auto* app = qobject_cast<QApplication*>(QCoreApplication::instance())) {
        return app;
    }

    static int argc = 1;
    static char appName[] = "FCComponentLib_tests_run";
    static char* argv[] = {appName, nullptr};
    static QApplication app(argc, argv);
    return &app;
}

}

TEST(ComponentRegistry, HasAtLeastOneRegisteredComponent)
{
    ensureApplication();

    const auto& components = FcComponents::ComponentRegistry::instance().all();
    EXPECT_FALSE(components.empty());
}

TEST(ComponentRegistry, CreatesAllRegisteredComponents)
{
    ensureApplication();

    const auto& components = FcComponents::ComponentRegistry::instance().all();
    ASSERT_FALSE(components.empty());

    for (const auto& component : components) {
        SCOPED_TRACE(component.name);
        QWidget* widget = FcComponents::ComponentRegistry::instance().create(component.name);
        ASSERT_NE(widget, nullptr);
        delete widget;
    }
}
