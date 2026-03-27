# SPDX-License-Identifier: LGPL-2.1-or-later

set(FileCardViewLayout_LIBS
    StartGui
)

set(FirstStartWidgetTest_LIBS
    StartGui
)

set(StartupWizardOverlayTest_LIBS
    StartGui
    FreeCADGui
)

setup_qt_test(FileCardViewLayout)
setup_qt_test(FirstStartWidgetTest)
setup_qt_test(StartupWizardOverlayTest)
