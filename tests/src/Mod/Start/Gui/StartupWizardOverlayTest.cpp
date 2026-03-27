// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QApplication>
#include <QDialog>
#include <QEvent>
#include <QPointer>
#include <QPushButton>
#include <QShortcut>
#include <QSignalSpy>
#include <QTest>
#include <QWidget>

#include <App/Application.h>
#include <Base/Parameter.h>
#include <Gui/MainWindow.h>
#include <Mod/Start/Gui/StartupWizardController.h>
#include <Mod/Start/Gui/StartupWizardOverlay.h>
#include <src/App/InitApplication.h>

namespace
{

bool isDescendantOf(const QWidget* widget, const QWidget* ancestor)
{
    for (auto current = widget; current != nullptr; current = current->parentWidget()) {
        if (current == ancestor) {
            return true;
        }
    }

    return false;
}

QKeySequence firstStandardBinding(QKeySequence::StandardKey standardKey)
{
    const auto bindings = QKeySequence::keyBindings(standardKey);
    return bindings.isEmpty() ? QKeySequence() : bindings.constFirst();
}

class TestStartupWizardController: public StartGui::StartupWizardController
{
public:
    explicit TestStartupWizardController(QWidget* mainWindow)
        : StartGui::StartupWizardController(nullptr)
        , testMainWindow(mainWindow)
    {}

    using StartGui::StartupWizardController::attemptStartup;

    QStringList commandsRun;
    bool commandsAvailable = true;
    QPointer<QWidget> focusBeforeCommand;
    QPointer<QWidget> focusOnCommandRun;
    QPointer<QDialog> modalToShowOnStart;

protected:
    QWidget* getMainWindow() const override
    {
        return testMainWindow;
    }

    bool canRunCommandByName(const char* /*name*/) const override
    {
        return commandsAvailable;
    }

    bool runCommandByName(const char* name) override
    {
        if (!commandsAvailable) {
            return false;
        }

        const QString commandName = QString::fromLatin1(name);
        commandsRun << commandName;

        if (commandName == QStringLiteral("Start_Start") && modalToShowOnStart) {
            modalToShowOnStart->show();
            qApp->processEvents();
        }

        if (commandName == QStringLiteral("Std_DlgPreferences")) {
            focusBeforeCommand = testMainWindow ? testMainWindow->focusWidget() : nullptr;
            if (focusOnCommandRun) {
                focusOnCommandRun->setFocus(Qt::OtherFocusReason);
            }
        }

        return true;
    }

private:
    QWidget* testMainWindow;
};

class testStartupWizardOverlay: public QObject
{
    Q_OBJECT

public:
    testStartupWizardOverlay()
    {
        tests::initApplication();
        startPreferences = App::GetApplication().GetParameterGroupByPath(
            "User parameter:BaseApp/Preferences/Mod/Start"
        );
        originalShowOnStartup = startPreferences->GetBool("ShowOnStartup", true);
        originalFirstStart = startPreferences->GetBool("FirstStart2024", true);
    }

    ~testStartupWizardOverlay() override
    {
        startPreferences->SetBool("ShowOnStartup", originalShowOnStartup);
        startPreferences->SetBool("FirstStart2024", originalFirstStart);
    }

private Q_SLOTS:
    void overlayCoversMainWindowAndStaysRaised()  // NOLINT
    {
        Gui::MainWindow mainWindow;
        mainWindow.resize(1280, 840);
        mainWindow.setProperty("eventLoop", true);
        mainWindow.show();
        qApp->processEvents();

        StartGui::StartupWizardOverlay overlay(&mainWindow);
        overlay.showOverlay();
        qApp->processEvents();

        QCOMPARE(overlay.parentWidget(), &mainWindow);
        QCOMPARE(overlay.geometry(), mainWindow.rect());

        auto* centeredChild = mainWindow.childAt(mainWindow.rect().center());
        QVERIFY(centeredChild);
        QVERIFY(isDescendantOf(centeredChild, &overlay));

        if (auto* mdiArea = mainWindow.getMdiArea()) {
            mdiArea->raise();
            qApp->processEvents();
        }

        centeredChild = mainWindow.childAt(mainWindow.rect().center());
        QVERIFY(centeredChild);
        QVERIFY(isDescendantOf(centeredChild, &overlay));

        mainWindow.resize(1140, 760);
        qApp->processEvents();
        QCOMPARE(overlay.geometry(), mainWindow.rect());

        QEvent styleChange(QEvent::StyleChange);
        qApp->sendEvent(&overlay, &styleChange);
        qApp->processEvents();

        centeredChild = mainWindow.childAt(mainWindow.rect().center());
        QVERIFY(centeredChild);
        QVERIFY(isDescendantOf(centeredChild, &overlay));
    }

    void scrimClickAndEscapeDoNotDismiss()  // NOLINT
    {
        Gui::MainWindow mainWindow;
        mainWindow.resize(1200, 800);
        mainWindow.setProperty("eventLoop", true);
        mainWindow.show();
        qApp->processEvents();

        StartGui::StartupWizardOverlay overlay(&mainWindow);
        overlay.showOverlay();
        qApp->processEvents();

        auto* primaryButton = overlay.findChild<QPushButton*>(QStringLiteral("firstStartPrimaryButton"));
        QVERIFY(primaryButton);

        QSignalSpy dismissedSpy(&overlay, &StartGui::StartupWizardOverlay::dismissed);

        QTest::mouseClick(&overlay, Qt::LeftButton, Qt::NoModifier, QPoint(8, 8));
        qApp->processEvents();
        QCOMPARE(dismissedSpy.count(), 0);
        QVERIFY(overlay.isVisible());

        QTest::keyClick(primaryButton, Qt::Key_Escape);
        qApp->processEvents();
        QCOMPARE(dismissedSpy.count(), 0);
        QVERIFY(overlay.isVisible());
    }

    void overlayBlocksNonAppShortcuts()  // NOLINT
    {
        Gui::MainWindow mainWindow;
        mainWindow.resize(1200, 800);
        mainWindow.setProperty("eventLoop", true);
        mainWindow.show();
        qApp->processEvents();

        int shortcutTriggered = 0;
        QShortcut shortcut(QKeySequence(QStringLiteral("Ctrl+J")), &mainWindow);
        shortcut.setContext(Qt::ApplicationShortcut);
        connect(&shortcut, &QShortcut::activated, this, [&shortcutTriggered]() { ++shortcutTriggered; });

        StartGui::StartupWizardOverlay overlay(&mainWindow);
        overlay.showOverlay();
        qApp->processEvents();

        auto* primaryButton = overlay.findChild<QPushButton*>(QStringLiteral("firstStartPrimaryButton"));
        QVERIFY(primaryButton);

        QTest::keySequence(primaryButton, QKeySequence(QStringLiteral("Ctrl+J")));
        qApp->processEvents();

        QCOMPARE(shortcutTriggered, 0);
    }

    void quitShortcutRemainsAvailable()  // NOLINT
    {
        const QKeySequence quitSequence = firstStandardBinding(QKeySequence::Quit);
        if (quitSequence.isEmpty()) {
            QSKIP("No quit shortcut binding on this platform");
        }

        Gui::MainWindow mainWindow;
        mainWindow.resize(1200, 800);
        mainWindow.setProperty("eventLoop", true);
        mainWindow.show();
        qApp->processEvents();

        int shortcutTriggered = 0;
        QShortcut shortcut(quitSequence, &mainWindow);
        shortcut.setContext(Qt::ApplicationShortcut);
        connect(&shortcut, &QShortcut::activated, this, [&shortcutTriggered]() { ++shortcutTriggered; });

        StartGui::StartupWizardOverlay overlay(&mainWindow);
        overlay.showOverlay();
        qApp->processEvents();

        auto* primaryButton = overlay.findChild<QPushButton*>(QStringLiteral("firstStartPrimaryButton"));
        QVERIFY(primaryButton);

        QTest::keySequence(primaryButton, quitSequence);
        qApp->processEvents();

        QCOMPARE(shortcutTriggered, 1);
    }

    void preferencesShortcutRemainsAvailable()  // NOLINT
    {
        const QKeySequence preferencesSequence = firstStandardBinding(QKeySequence::Preferences);
        if (preferencesSequence.isEmpty()) {
            QSKIP("No preferences shortcut binding on this platform");
        }

        Gui::MainWindow mainWindow;
        mainWindow.resize(1200, 800);
        mainWindow.setProperty("eventLoop", true);
        mainWindow.show();
        qApp->processEvents();

        int shortcutTriggered = 0;
        QShortcut shortcut(preferencesSequence, &mainWindow);
        shortcut.setContext(Qt::ApplicationShortcut);
        connect(&shortcut, &QShortcut::activated, this, [&shortcutTriggered]() { ++shortcutTriggered; });

        StartGui::StartupWizardOverlay overlay(&mainWindow);
        overlay.showOverlay();
        qApp->processEvents();

        auto* primaryButton = overlay.findChild<QPushButton*>(QStringLiteral("firstStartPrimaryButton"));
        QVERIFY(primaryButton);

        QTest::keySequence(primaryButton, preferencesSequence);
        qApp->processEvents();

        QCOMPARE(shortcutTriggered, 1);
    }

    void closeShortcutRemainsAvailable()  // NOLINT
    {
        const QKeySequence closeSequence = firstStandardBinding(QKeySequence::Close);
        if (closeSequence.isEmpty()) {
            QSKIP("No close shortcut binding on this platform");
        }

        Gui::MainWindow mainWindow;
        mainWindow.resize(1200, 800);
        mainWindow.setProperty("eventLoop", true);
        mainWindow.show();
        qApp->processEvents();

        int shortcutTriggered = 0;
        QShortcut shortcut(closeSequence, &mainWindow);
        shortcut.setContext(Qt::ApplicationShortcut);
        connect(&shortcut, &QShortcut::activated, this, [&shortcutTriggered]() { ++shortcutTriggered; });

        StartGui::StartupWizardOverlay overlay(&mainWindow);
        overlay.showOverlay();
        qApp->processEvents();

        auto* primaryButton = overlay.findChild<QPushButton*>(QStringLiteral("firstStartPrimaryButton"));
        QVERIFY(primaryButton);

        QTest::keySequence(primaryButton, closeSequence);
        qApp->processEvents();

        QCOMPARE(shortcutTriggered, 1);
    }

    void startupOpensStartPageAndOverlayWhenConfigured()  // NOLINT
    {
        startPreferences->SetBool("ShowOnStartup", true);
        startPreferences->SetBool("FirstStart2024", true);

        Gui::MainWindow mainWindow;
        mainWindow.resize(1280, 840);
        mainWindow.setProperty("eventLoop", true);
        mainWindow.show();
        qApp->processEvents();

        TestStartupWizardController controller(&mainWindow);
        controller.attemptStartup();
        qApp->processEvents();

        QCOMPARE(controller.commandsRun, QStringList {QStringLiteral("Start_Start")});
        QVERIFY(controller.overlay());
        QVERIFY(controller.overlay()->isVisible());
    }

    void startupShowsOverlayWithoutStartPageWhenDisabled()  // NOLINT
    {
        startPreferences->SetBool("ShowOnStartup", false);
        startPreferences->SetBool("FirstStart2024", true);

        Gui::MainWindow mainWindow;
        mainWindow.resize(1280, 840);
        mainWindow.setProperty("eventLoop", true);
        mainWindow.show();
        qApp->processEvents();

        TestStartupWizardController controller(&mainWindow);
        controller.attemptStartup();
        qApp->processEvents();

        QVERIFY(controller.commandsRun.isEmpty());
        QVERIFY(controller.overlay());
        QVERIFY(controller.overlay()->isVisible());
    }

    void startupSkipsOverlayWhenFirstStartCompleted()  // NOLINT
    {
        startPreferences->SetBool("ShowOnStartup", true);
        startPreferences->SetBool("FirstStart2024", false);

        Gui::MainWindow mainWindow;
        mainWindow.resize(1280, 840);
        mainWindow.setProperty("eventLoop", true);
        mainWindow.show();
        qApp->processEvents();

        TestStartupWizardController controller(&mainWindow);
        controller.attemptStartup();
        qApp->processEvents();

        QCOMPARE(controller.commandsRun, QStringList {QStringLiteral("Start_Start")});
        QVERIFY(!controller.overlay());
    }

    void manualReopenAndDismissUseControllerState()  // NOLINT
    {
        startPreferences->SetBool("ShowOnStartup", false);
        startPreferences->SetBool("FirstStart2024", false);

        Gui::MainWindow mainWindow;
        mainWindow.resize(1280, 840);
        mainWindow.setProperty("eventLoop", true);
        mainWindow.show();
        qApp->processEvents();

        TestStartupWizardController controller(&mainWindow);
        controller.showManual();
        qApp->processEvents();

        QVERIFY(controller.overlay());
        QVERIFY(controller.overlay()->isVisible());

        auto* primaryButton =
            controller.overlay()->findChild<QPushButton*>(QStringLiteral("firstStartPrimaryButton"));
        QVERIFY(primaryButton);

        QTest::mouseClick(primaryButton, Qt::LeftButton);
        qApp->processEvents();

        QVERIFY(!controller.overlay()->isVisible());
        QVERIFY(!startPreferences->GetBool("FirstStart2024", true));
    }

    void hiddenOverlayStopsBlockingShortcuts()  // NOLINT
    {
        startPreferences->SetBool("ShowOnStartup", false);
        startPreferences->SetBool("FirstStart2024", false);

        Gui::MainWindow mainWindow;
        mainWindow.resize(1280, 840);
        mainWindow.setProperty("eventLoop", true);
        mainWindow.show();
        qApp->processEvents();

        int shortcutTriggered = 0;
        QShortcut shortcut(QKeySequence(QStringLiteral("Ctrl+J")), &mainWindow);
        shortcut.setContext(Qt::ApplicationShortcut);
        connect(&shortcut, &QShortcut::activated, this, [&shortcutTriggered]() { ++shortcutTriggered; });

        TestStartupWizardController controller(&mainWindow);
        controller.showManual();
        qApp->processEvents();

        auto* primaryButton =
            controller.overlay()->findChild<QPushButton*>(QStringLiteral("firstStartPrimaryButton"));
        QVERIFY(primaryButton);

        QTest::keySequence(primaryButton, QKeySequence(QStringLiteral("Ctrl+J")));
        qApp->processEvents();
        QCOMPARE(shortcutTriggered, 0);

        QTest::mouseClick(primaryButton, Qt::LeftButton);
        qApp->processEvents();

        QVERIFY(!controller.overlay()->isVisible());

        QTest::keySequence(&mainWindow, QKeySequence(QStringLiteral("Ctrl+J")));
        qApp->processEvents();

        QCOMPARE(shortcutTriggered, 1);
    }

    void advancedSettingsClosesOverlayWithoutClearingFirstStart()  // NOLINT
    {
        startPreferences->SetBool("ShowOnStartup", false);
        startPreferences->SetBool("FirstStart2024", true);

        Gui::MainWindow mainWindow;
        mainWindow.resize(1280, 840);
        mainWindow.setProperty("eventLoop", true);
        mainWindow.show();
        qApp->processEvents();

        QPushButton priorFocus(QStringLiteral("prior"), &mainWindow);
        priorFocus.setGeometry(20, 20, 80, 24);
        priorFocus.show();
        priorFocus.setFocus(Qt::OtherFocusReason);

        QPushButton preferencesFocus(QStringLiteral("preferences"), &mainWindow);
        preferencesFocus.setGeometry(120, 20, 100, 24);
        preferencesFocus.show();

        TestStartupWizardController controller(&mainWindow);
        controller.focusOnCommandRun = &preferencesFocus;
        controller.showManual();
        qApp->processEvents();

        auto* advancedButton =
            controller.overlay()->findChild<QPushButton*>(QStringLiteral("firstStartSecondaryButton"));
        QVERIFY(advancedButton);

        QTest::mouseClick(advancedButton, Qt::LeftButton);
        qApp->processEvents();

        QCOMPARE(controller.commandsRun, QStringList {QStringLiteral("Std_DlgPreferences")});
        QVERIFY(controller.focusBeforeCommand != &priorFocus);
        QVERIFY(mainWindow.focusWidget() == &preferencesFocus);
        QVERIFY(!controller.overlay()->isVisible());
        QVERIFY(startPreferences->GetBool("FirstStart2024", false));
    }

    void manualShowDoesNotAppearOverActiveModalDialog()  // NOLINT
    {
        startPreferences->SetBool("ShowOnStartup", false);
        startPreferences->SetBool("FirstStart2024", false);

        Gui::MainWindow mainWindow;
        mainWindow.resize(1280, 840);
        mainWindow.setProperty("eventLoop", true);
        mainWindow.show();
        qApp->processEvents();

        QDialog modal(&mainWindow);
        modal.setWindowModality(Qt::ApplicationModal);
        modal.show();
        qApp->processEvents();

        QVERIFY(qApp->activeModalWidget() == &modal);

        TestStartupWizardController controller(&mainWindow);
        controller.showManual();
        qApp->processEvents();

        QVERIFY(!controller.overlay());
    }

    void startupDefersOverlayUntilPostStartModalClears()  // NOLINT
    {
        startPreferences->SetBool("ShowOnStartup", true);
        startPreferences->SetBool("FirstStart2024", true);

        Gui::MainWindow mainWindow;
        mainWindow.resize(1280, 840);
        mainWindow.setProperty("eventLoop", true);
        mainWindow.show();
        qApp->processEvents();

        QDialog modal(&mainWindow);
        modal.setWindowModality(Qt::ApplicationModal);

        TestStartupWizardController controller(&mainWindow);
        controller.modalToShowOnStart = &modal;
        controller.attemptStartup();
        qApp->processEvents();

        QCOMPARE(controller.commandsRun, QStringList {QStringLiteral("Start_Start")});
        QVERIFY(qApp->activeModalWidget() == &modal);
        QVERIFY(!controller.overlay());

        modal.hide();
        qApp->processEvents();

        controller.attemptStartup();
        qApp->processEvents();

        QVERIFY(controller.overlay());
        QVERIFY(controller.overlay()->isVisible());
    }

private:
    Base::Reference<ParameterGrp> startPreferences;
    bool originalShowOnStartup = true;
    bool originalFirstStart = true;
};

}  // namespace

QTEST_MAIN(testStartupWizardOverlay)

#include "StartupWizardOverlayTest.moc"
