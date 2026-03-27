# SPDX-License-Identifier: LGPL-2.1-or-later

cmake_minimum_required(VERSION 3.22)

# Standalone build detection
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    project(FCComponentLib VERSION 0.1 LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    find_package(Qt6 6.8 REQUIRED COMPONENTS Core Widgets)
endif()

set(CMAKE_AUTOMOC TRUE)

# Component infrastructure
set(FCComponentLib_Components_SRCS
    Components/ComponentMeta.h
    Components/ComponentRegistry.h
    Components/ComponentRegistry.cpp
)

# Minimal palette helpers kept for the gallery shell.
set(FCComponentLib_Style_SRCS
    Style/ComponentPalette.h
    Style/ComponentPalette.cpp
)

# Widget sources
set(FCComponentLib_Widgets_SRCS
    Components/Buttons/FcPushButton.h
    Components/Buttons/FcPushButton.cpp
)

add_library(FCComponentLib SHARED
    FCComponentLibGlobal.h
    ${FCComponentLib_Components_SRCS}
    ${FCComponentLib_Style_SRCS}
    ${FCComponentLib_Widgets_SRCS}
)

target_compile_definitions(FCComponentLib PRIVATE FCComponentLib_EXPORTS)

target_link_libraries(FCComponentLib
    PUBLIC Qt6::Core Qt6::Widgets
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
