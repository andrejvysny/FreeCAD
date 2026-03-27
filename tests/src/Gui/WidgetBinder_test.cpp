// SPDX-License-Identifier: LGPL-2.1-or-later

#include <gtest/gtest.h>

#include <utility>

#include <QApplication>
#include <QBuffer>
#include <QCoreApplication>
#include <QProcess>
#include <QProcessEnvironment>
#include <QWidget>

#include <FCComponentLib/Components/Buttons/FcPushButton.h>

#include <Gui/Adapters/AdapterRegistration.h>
#include <Gui/Adapters/WidgetBinder.h>
#include <Gui/UiLoader.h>
#include <Gui/WidgetFactory.h>

namespace
{

QApplication* ensureApplication()
{
    if (auto* app = qobject_cast<QApplication*>(QCoreApplication::instance())) {
        return app;
    }

    static int argc = 1;
    static char appName[] = "Gui_tests_run";
    static char* argv[] = {appName, nullptr};
    static QApplication app(argc, argv);
    return &app;
}

class AdapterRegistrationGuard
{
public:
    explicit AdapterRegistrationGuard(const char* widgetType)
        : m_widgetType(widgetType)
    {
        Gui::Adapters::AdapterHooks hooks;
        hooks.onAttach = [](QWidget& widget) {
            widget.setProperty("fcTestAdapterAttachCalled", true);
        };
        Gui::Adapters::AdapterRegistration::instance().registerAdapter(m_widgetType, std::move(hooks));
    }

    ~AdapterRegistrationGuard()
    {
        Gui::Adapters::AdapterRegistration::instance().unregisterAdapter(m_widgetType);
    }

private:
    const char* m_widgetType;
};

}

TEST(WidgetFactoryCompatibility, CreatesFCComponentAndAttachesAdapter)
{
    ensureApplication();
    AdapterRegistrationGuard guard(FcComponents::FcPushButton::staticMetaObject.className());

    QWidget* widget = Gui::WidgetFactory().createWidget(
        FcComponents::FcPushButton::staticMetaObject.className()
    );

    ASSERT_NE(widget, nullptr);
    EXPECT_TRUE(widget->property("fcTestAdapterAttachCalled").toBool());
    EXPECT_TRUE(widget->property("_fcGuiAdapterBound").toBool());

    EXPECT_TRUE(Gui::Adapters::WidgetBinder::instance().detach(widget));
    EXPECT_FALSE(widget->property("_fcGuiAdapterBound").toBool());
    delete widget;
}

TEST(UiLoaderCompatibility, LoadsUiCustomWidgetAndAttachesAdapter)
{
    ensureApplication();
    AdapterRegistrationGuard guard(FcComponents::FcPushButton::staticMetaObject.className());

    static constexpr auto uiXml = R"(<ui version="4.0">
 <class>Form</class>
 <widget class="QWidget" name="Form">
  <layout class="QVBoxLayout" name="verticalLayout">
   <item>
    <widget class="FcPushButton" name="testButton">
     <property name="text">
      <string>Click</string>
     </property>
    </widget>
   </item>
  </layout>
 </widget>
 <customwidgets>
  <customwidget>
   <class>FcPushButton</class>
   <extends>QPushButton</extends>
   <header>FCComponentLib/Components/Buttons/FcPushButton.h</header>
  </customwidget>
 </customwidgets>
</ui>)";

    QBuffer buffer;
    buffer.setData(uiXml);
    ASSERT_TRUE(buffer.open(QIODevice::ReadOnly));

    std::unique_ptr<Gui::UiLoader> loader = Gui::UiLoader::newInstance();
    QWidget* form = loader->load(&buffer, nullptr);
    ASSERT_NE(form, nullptr);

    auto* button = form->findChild<FcComponents::FcPushButton*>("testButton");
    ASSERT_NE(button, nullptr);
    EXPECT_TRUE(button->property("fcTestAdapterAttachCalled").toBool());
    EXPECT_TRUE(button->property("_fcGuiAdapterBound").toBool());

    delete form;
}

TEST(GallerySmoke, LaunchesAndStartsEventLoop)
{
    ensureApplication();

#ifdef FC_GALLERY_EXECUTABLE
    QProcess process;
    process.setProgram(QString::fromUtf8(FC_GALLERY_EXECUTABLE));
    process.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    process.start();

    ASSERT_TRUE(process.waitForStarted(5000));
    EXPECT_EQ(process.state(), QProcess::Running);

    process.terminate();
    if (!process.waitForFinished(5000)) {
        process.kill();
        process.waitForFinished(5000);
    }
#else
    GTEST_SKIP() << "FCComponentGallery target is not available in this build";
#endif
}
