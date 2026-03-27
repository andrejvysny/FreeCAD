# SPDX-License-Identifier: LGPL-2.1-or-later

add_executable(FCComponentLib_tests_run
    ComponentRegistry_test.cpp
    ForbiddenInclude_test.cpp
)

target_compile_definitions(FCComponentLib_tests_run PRIVATE
    FC_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(FCComponentLib_tests_run PRIVATE
    GTest::gtest_main
    GTest::gmock_main
    ${Google_Tests_LIBS}
    FCComponentLib
    Qt6::Core
    Qt6::Widgets
)
