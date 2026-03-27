// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QApplication>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include <algorithm>
#include <limits>
#include <string>

#include <App/Application.h>
#include <Base/Parameter.h>
#include <Mod/Start/Gui/FirstStartWidget.h>
#include <Mod/Start/Gui/ThemeSelectorWidget.h>
#include <src/App/InitApplication.h>

namespace
{

void restoreAsciiParameter(const Base::Reference<ParameterGrp>& group,
                           const char* name,
                           const std::string& value)
{
    if (value.empty()) {
        group->RemoveASCII(name);
    }
    else {
        group->SetASCII(name, value);
    }
}

void restoreUnsignedParameter(const Base::Reference<ParameterGrp>& group,
                              const char* name,
                              unsigned long value,
                              unsigned long missingValue)
{
    if (value == missingValue) {
        group->RemoveUnsigned(name);
    }
    else {
        group->SetUnsigned(name, value);
    }
}

void restoreIntParameter(const Base::Reference<ParameterGrp>& group,
                         const char* name,
                         int value,
                         int missingValue)
{
    if (value == missingValue) {
        group->RemoveInt(name);
    }
    else {
        group->SetInt(name, value);
    }
}

int nextIndex(const QComboBox* comboBox)
{
    if (!comboBox || comboBox->count() < 2) {
        return comboBox ? comboBox->currentIndex() : 0;
    }

    return (comboBox->currentIndex() + 1) % comboBox->count();
}

class testFirstStartWidget: public QObject
{
    Q_OBJECT

public:
    testFirstStartWidget()
    {
        tests::initApplication();

        unitsPreferences = App::GetApplication().GetParameterGroupByPath(
            "User parameter:BaseApp/Preferences/Units"
        );
        viewPreferences = App::GetApplication().GetParameterGroupByPath(
            "User parameter:BaseApp/Preferences/View"
        );
        mainWindowPreferences = App::GetApplication().GetParameterGroupByPath(
            "User parameter:BaseApp/Preferences/MainWindow"
        );
        themePreferences = App::GetApplication().GetParameterGroupByPath(
            "User parameter:BaseApp/Preferences/Themes"
        );

        originalUnitSchema = unitsPreferences->GetInt("UserSchema", 0);
        originalNavigationStyle = viewPreferences->GetASCII("NavigationStyle", nullptr);
        originalOrbitStyle = viewPreferences->GetInt("OrbitStyle", missingInt);
        originalTheme = mainWindowPreferences->GetASCII("Theme", nullptr);
        originalStyleSheet = mainWindowPreferences->GetASCII("StyleSheet", nullptr);
        originalAccentColor1 = themePreferences->GetUnsigned("ThemeAccentColor1", missingUnsigned);
        originalAccentColor2 = themePreferences->GetUnsigned("ThemeAccentColor2", missingUnsigned);
        originalAccentColor3 = themePreferences->GetUnsigned("ThemeAccentColor3", missingUnsigned);
    }

    ~testFirstStartWidget() override
    {
        unitsPreferences->SetInt("UserSchema", originalUnitSchema);
        restoreAsciiParameter(viewPreferences, "NavigationStyle", originalNavigationStyle);
        restoreIntParameter(viewPreferences, "OrbitStyle", originalOrbitStyle, missingInt);
        restoreAsciiParameter(mainWindowPreferences, "Theme", originalTheme);
        restoreAsciiParameter(mainWindowPreferences, "StyleSheet", originalStyleSheet);
        restoreUnsignedParameter(
            themePreferences,
            "ThemeAccentColor1",
            originalAccentColor1,
            missingUnsigned
        );
        restoreUnsignedParameter(
            themePreferences,
            "ThemeAccentColor2",
            originalAccentColor2,
            missingUnsigned
        );
        restoreUnsignedParameter(
            themePreferences,
            "ThemeAccentColor3",
            originalAccentColor3,
            missingUnsigned
        );
    }

private Q_SLOTS:
    void headerAndSectionOrder()  // NOLINT
    {
        StartGui::FirstStartWidget widget;
        widget.resize(860, 520);
        widget.show();
        qApp->processEvents();

        auto* wordmark = widget.findChild<QLabel*>(QStringLiteral("firstStartWordmark"));
        auto* intro = widget.findChild<QLabel*>(QStringLiteral("firstStartIntro"));
        auto* language = widget.findChild<QLabel*>(QStringLiteral("firstStartLanguageLabel"));
        auto* appearance = widget.findChild<QLabel*>(QStringLiteral("firstStartAppearanceLabel"));
        auto* actionRow = widget.findChild<QWidget*>(QStringLiteral("firstStartActionRow"));

        QVERIFY(wordmark);
        QVERIFY(intro);
        QVERIFY(language);
        QVERIFY(appearance);
        QVERIFY(actionRow);

        QVERIFY(!widget.findChild<QLabel*>(QStringLiteral("firstStartTitle")));
        QVERIFY(!widget.findChild<QLabel*>(QStringLiteral("firstStartCoreSetupLabel")));
        QVERIFY(!widget.findChild<QLabel*>(QStringLiteral("firstStartPersonalizationLabel")));
        QVERIFY(!widget.findChild<QLabel*>(QStringLiteral("firstStartBasicsLabel")));
        QVERIFY(!widget.findChild<QLabel*>(QStringLiteral("firstStartControlsLabel")));
        QVERIFY(wordmark->geometry().top() < intro->geometry().top());
        QVERIFY(intro->geometry().top() < language->geometry().top());
        QVERIFY(language->geometry().top() < appearance->geometry().top());
        QVERIFY(appearance->geometry().top() < actionRow->geometry().top());
    }

    void labelsAndHelperTextArePresent()  // NOLINT
    {
        StartGui::FirstStartWidget widget;

        QVERIFY(!widget.findChild<QLabel*>(QStringLiteral("firstStartTitle")));
        QVERIFY(!widget.findChild<QLabel*>(QStringLiteral("firstStartCoreSetupLabel")));
        QVERIFY(!widget.findChild<QLabel*>(QStringLiteral("firstStartPersonalizationLabel")));
        QVERIFY(!widget.findChild<QLabel*>(QStringLiteral("firstStartBasicsLabel")));
        QVERIFY(!widget.findChild<QLabel*>(QStringLiteral("firstStartControlsLabel")));
        QCOMPARE(
            widget.findChild<QLabel*>(QStringLiteral("firstStartOrbitStyleLabel"))->text(),
            QStringLiteral("Orbit style")
        );
        QCOMPARE(
            widget.findChild<QLabel*>(QStringLiteral("firstStartAppearanceLabel"))->text(),
            QStringLiteral("Appearance")
        );
        QCOMPARE(
            widget.findChild<QLabel*>(QStringLiteral("firstStartLanguageHelperText"))->text(),
            QStringLiteral("Used for UI text")
        );
        QCOMPARE(
            widget.findChild<QLabel*>(QStringLiteral("firstStartUnitsHelperText"))->text(),
            QStringLiteral("Affects dimensions and measurements")
        );
        QCOMPARE(
            widget.findChild<QLabel*>(QStringLiteral("firstStartNavigationHelperText"))->text(),
            QStringLiteral("Controls mouse behavior")
        );
        QCOMPARE(
            widget.findChild<QLabel*>(QStringLiteral("firstStartOrbitStyleHelperText"))->text(),
            QStringLiteral("Controls rotation behavior")
        );
        QVERIFY(widget.findChild<QComboBox*>(QStringLiteral("firstStartOrbitStyleComboBox")));
        QCOMPARE(
            widget.findChild<QPushButton*>(QStringLiteral("firstStartPrimaryButton"))->text(),
            QStringLiteral("Start Using FreeCAD")
        );
        QCOMPARE(
            widget.findChild<QPushButton*>(QStringLiteral("firstStartSecondaryButton"))->text(),
            QStringLiteral("Advanced Settings")
        );
    }

    void primaryCtaEmitsDismissed()  // NOLINT
    {
        StartGui::FirstStartWidget widget;
        auto* primaryButton = widget.findChild<QPushButton*>(QStringLiteral("firstStartPrimaryButton"));
        QVERIFY(primaryButton);

        QSignalSpy dismissedSpy(&widget, &StartGui::FirstStartWidget::dismissed);
        QTest::mouseClick(primaryButton, Qt::LeftButton);

        QCOMPARE(dismissedSpy.count(), 1);
    }

    void advancedSettingsRequestDoesNotDismiss()  // NOLINT
    {
        StartGui::FirstStartWidget widget;
        widget.resize(860, 520);
        widget.show();
        qApp->processEvents();

        auto* advancedButton
            = widget.findChild<QPushButton*>(QStringLiteral("firstStartSecondaryButton"));

        QVERIFY(advancedButton);

        QSignalSpy dismissedSpy(&widget, &StartGui::FirstStartWidget::dismissed);
        QSignalSpy advancedSpy(&widget, &StartGui::FirstStartWidget::advancedSettingsRequested);
        QTest::mouseClick(advancedButton, Qt::LeftButton);

        QCOMPARE(dismissedSpy.count(), 0);
        QCOMPARE(advancedSpy.count(), 1);
    }

    void refreshFromPreferencesUpdatesSelections()  // NOLINT
    {
        StartGui::FirstStartWidget widget;
        widget.resize(860, 520);
        widget.show();
        qApp->processEvents();

        auto* unitCombo
            = widget.findChild<QComboBox*>(QStringLiteral("firstStartUnitSystemComboBox"));
        auto* navigationCombo
            = widget.findChild<QComboBox*>(QStringLiteral("firstStartNavigationStyleComboBox"));
        auto* orbitCombo
            = widget.findChild<QComboBox*>(QStringLiteral("firstStartOrbitStyleComboBox"));
        auto* darkButton
            = widget.findChild<QToolButton*>(QStringLiteral("firstStartThemeDarkButton"));
        auto* lightButton
            = widget.findChild<QToolButton*>(QStringLiteral("firstStartThemeLightButton"));

        QVERIFY(unitCombo);
        QVERIFY(navigationCombo);
        QVERIFY(orbitCombo);
        QVERIFY(darkButton);
        QVERIFY(lightButton);

        const int targetUnitSchema = nextIndex(unitCombo);
        const int navigationIndex = nextIndex(navigationCombo);
        const QByteArray targetNavigationStyle
            = navigationCombo->itemData(navigationIndex).toByteArray();
        const int targetOrbitStyle = nextIndex(orbitCombo);
        const std::string targetThemeName = darkButton->isChecked() ? "FreeCAD Light" : "FreeCAD Dark";

        unitsPreferences->SetInt("UserSchema", targetUnitSchema);
        viewPreferences->SetASCII("NavigationStyle", targetNavigationStyle.constData());
        viewPreferences->SetInt("OrbitStyle", targetOrbitStyle);
        mainWindowPreferences->SetASCII("Theme", targetThemeName);

        widget.refreshFromPreferences();

        QCOMPARE(unitCombo->currentIndex(), targetUnitSchema);
        QCOMPARE(navigationCombo->currentData().toByteArray(), targetNavigationStyle);
        QCOMPARE(orbitCombo->currentData().toInt(), targetOrbitStyle);
        QCOMPARE(darkButton->isChecked(), targetThemeName == "FreeCAD Dark");
        QCOMPARE(lightButton->isChecked(), targetThemeName == "FreeCAD Light");
    }

    void wordmarkStaysPopulatedOnStyleRefresh()  // NOLINT
    {
        StartGui::FirstStartWidget widget;
        widget.resize(860, 520);
        widget.show();
        qApp->processEvents();

        auto* wordmark = widget.findChild<QLabel*>(QStringLiteral("firstStartWordmark"));
        QVERIFY(wordmark);
        QVERIFY(!wordmark->pixmap(Qt::ReturnByValue).isNull());

        QEvent styleChange(QEvent::StyleChange);
        QApplication::sendEvent(&widget, &styleChange);

        QVERIFY(!wordmark->pixmap(Qt::ReturnByValue).isNull());
    }

    void widgetStaysCompactInHostLayout()  // NOLINT
    {
        QWidget host;
        auto* hostLayout = new QVBoxLayout(&host);
        hostLayout->setContentsMargins(24, 24, 24, 24);
        hostLayout->setSpacing(0);
        hostLayout->addStretch(1);

        auto* centeredRow = new QHBoxLayout();
        centeredRow->setContentsMargins(0, 0, 0, 0);
        centeredRow->setSpacing(0);
        centeredRow->addStretch();

        StartGui::FirstStartWidget widget(&host);
        centeredRow->addWidget(&widget);
        centeredRow->addStretch();

        hostLayout->addLayout(centeredRow);
        hostLayout->addStretch(1);

        host.resize(1400, 900);
        host.show();
        qApp->processEvents();

        QVERIFY(widget.width() <= 760);
        QVERIFY(widget.height() <= widget.sizeHint().height() + 4);

        const QMargins margins = hostLayout->contentsMargins();
        const int topFreeSpace = widget.geometry().top() - margins.top();
        const int bottomFreeSpace
            = host.height() - margins.bottom() - widget.geometry().bottom() - 1;
        const int difference
            = topFreeSpace > bottomFreeSpace ? topFreeSpace - bottomFreeSpace
                                             : bottomFreeSpace - topFreeSpace;
        QVERIFY(difference <= 4);
    }

    void themeCardsStayVisibleInOneRow()  // NOLINT
    {
        StartGui::ThemeSelectorWidget widget;
        widget.resize(520, 240);
        widget.show();
        qApp->processEvents();

        auto* lightButton
            = widget.findChild<QToolButton*>(QStringLiteral("firstStartThemeLightButton"));
        auto* darkButton = widget.findChild<QToolButton*>(QStringLiteral("firstStartThemeDarkButton"));
        auto* classicButton
            = widget.findChild<QToolButton*>(QStringLiteral("firstStartThemeClassicButton"));

        QVERIFY(classicButton);
        QVERIFY(lightButton);
        QVERIFY(darkButton);
        QVERIFY(classicButton->isVisibleTo(&widget));
        QVERIFY(lightButton->isVisibleTo(&widget));
        QVERIFY(darkButton->isVisibleTo(&widget));
        QVERIFY(classicButton->geometry().isValid());
        QVERIFY(lightButton->geometry().isValid());
        QVERIFY(darkButton->geometry().isValid());

        auto* cardsWidget = lightButton->parentWidget();
        QVERIFY(cardsWidget);

        const QRect cardsRect(QPoint(0, 0), cardsWidget->size());
        QVERIFY(cardsRect.contains(classicButton->geometry()));
        QVERIFY(cardsRect.contains(lightButton->geometry()));
        QVERIFY(cardsRect.contains(darkButton->geometry()));
        QCOMPARE(classicButton->geometry().top(), lightButton->geometry().top());
        QCOMPARE(lightButton->geometry().top(), darkButton->geometry().top());
        QVERIFY(classicButton->geometry().right() < lightButton->geometry().left());
        QVERIFY(lightButton->geometry().right() < darkButton->geometry().left());

        int groupLeft = std::numeric_limits<int>::max();
        int groupRight = std::numeric_limits<int>::min();
        for (auto* button : {classicButton, lightButton, darkButton}) {
            if (button && button->isVisibleTo(cardsWidget)) {
                groupLeft = std::min(groupLeft, button->geometry().left());
                groupRight = std::max(groupRight, button->geometry().right());
            }
        }

        QVERIFY(groupLeft <= groupRight);
        const int groupCenter = (groupLeft + groupRight) / 2;
        const int cardsCenter = cardsRect.center().x();
        const int centerDifference = groupCenter > cardsCenter ? groupCenter - cardsCenter
                                                               : cardsCenter - groupCenter;
        QVERIFY(centerDifference <= 10);
    }

private:
    static constexpr unsigned long missingUnsigned = std::numeric_limits<unsigned long>::max();
    static constexpr int missingInt = std::numeric_limits<int>::max();

    Base::Reference<ParameterGrp> unitsPreferences;
    Base::Reference<ParameterGrp> viewPreferences;
    Base::Reference<ParameterGrp> mainWindowPreferences;
    Base::Reference<ParameterGrp> themePreferences;
    long originalUnitSchema = 0;
    std::string originalNavigationStyle;
    int originalOrbitStyle = missingInt;
    std::string originalTheme;
    std::string originalStyleSheet;
    unsigned long originalAccentColor1 = missingUnsigned;
    unsigned long originalAccentColor2 = missingUnsigned;
    unsigned long originalAccentColor3 = missingUnsigned;
};

}  // namespace

QTEST_MAIN(testFirstStartWidget)

#include "FirstStartWidgetTest.moc"
