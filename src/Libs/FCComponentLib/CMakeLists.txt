# SPDX-License-Identifier: LGPL-2.1-or-later

cmake_minimum_required(VERSION 3.22)

# Standalone build detection
if(NOT TARGET FreeCADBase)
    project(FCComponentLib VERSION 0.1 LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    find_package(Qt6 6.8 REQUIRED COMPONENTS Core Widgets Svg)
    find_package(yaml-cpp REQUIRED)
    find_package(fmt REQUIRED)
    # FreeCADBase is required for Base::Color, Base::Exception
    find_package(FreeCADBase REQUIRED)
endif()

set(CMAKE_AUTOMOC TRUE)

# Token system sources (extracted from Gui/StyleParameters)
set(FCComponentLib_Tokens_SRCS
    Tokens/Value.h
    Tokens/Value.cpp
    Tokens/Parser.h
    Tokens/Parser.cpp
    Tokens/TokenManager.h
    Tokens/TokenManager.cpp
    Tokens/TokenSource.h
    Tokens/TokenSource.cpp
)

# Style system sources
set(FCComponentLib_Style_SRCS
    Style/ComponentStyle.h
    Style/ComponentStyle.cpp
    Style/ComponentPalette.h
    Style/ComponentPalette.cpp
)

# Component infrastructure
set(FCComponentLib_Components_SRCS
    Components/ComponentMeta.h
    Components/ComponentRegistry.h
    Components/ComponentRegistry.cpp
)

# Widget sources
set(FCComponentLib_Widgets_SRCS
    Components/Buttons/FcPushButton.h
    Components/Buttons/FcPushButton.cpp
    Components/Inputs/FcSpinBox.h
    Components/Inputs/FcSpinBox.cpp
    Components/Inputs/FcDoubleSpinBox.h
    Components/Inputs/FcDoubleSpinBox.cpp
    Components/Inputs/FcLineEdit.h
    Components/Inputs/FcLineEdit.cpp
    Components/Inputs/FcComboBox.h
    Components/Inputs/FcComboBox.cpp
    Components/Feedback/FcColorButton.h
    Components/Feedback/FcColorButton.cpp
)

add_library(FCComponentLib SHARED
    FCComponentLibGlobal.h
    ${FCComponentLib_Tokens_SRCS}
    ${FCComponentLib_Style_SRCS}
    ${FCComponentLib_Components_SRCS}
    ${FCComponentLib_Widgets_SRCS}
)

target_compile_definitions(FCComponentLib PRIVATE FCComponentLib_EXPORTS)

target_link_libraries(FCComponentLib
    PUBLIC Qt6::Core Qt6::Widgets Qt6::Svg FreeCADBase
    PRIVATE yaml-cpp::yaml-cpp fmt::fmt
)

target_include_directories(FCComponentLib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

SET_BIN_DIR(FCComponentLib FCComponentLib)

install(TARGETS FCComponentLib DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Install public headers
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/FCComponentLib
    FILES_MATCHING PATTERN "*.h"
)

# Install token YAML files as data
install(DIRECTORY Tokens/tokens/ DESTINATION share/FCComponentLib/tokens)
