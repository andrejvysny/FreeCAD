# SPDX-License-Identifier: LGPL-2.1-or-later

option(BUILD_COMPONENT_GALLERY "Build the standalone component gallery application" ON)

if(NOT BUILD_COMPONENT_GALLERY)
    return()
endif()

set(CMAKE_AUTOMOC TRUE)

set(FCComponentGallery_SRCS
    main.cpp
    FlowLayout.h
    FlowLayout.cpp
    ToggleSwitch.h
    ToggleSwitch.cpp
    GalleryWindow.h
    GalleryWindow.cpp
    ComponentBrowser.h
    ComponentBrowser.cpp
    ComponentCanvas.h
    ComponentCanvas.cpp
    PropertyPanel.h
    PropertyPanel.cpp
    StoryPanel.h
    StoryPanel.cpp
    CodePreview.h
    CodePreview.cpp
    ThemeSwitcher.h
    ThemeSwitcher.cpp
    Stories/Story.h
    Stories/StoryRegistry.h
    Stories/StoryRegistry.cpp
    Stories/ButtonStories.cpp
    Stories/InputStories.cpp
    Stories/FeedbackStories.cpp
    Stories/DisplayStories.cpp
    Stories/ContainerStories.cpp
)

add_executable(FCComponentGallery ${FCComponentGallery_SRCS})

target_link_libraries(FCComponentGallery PRIVATE
    FCComponentLib
    Qt6::Core
    Qt6::Widgets
)

target_include_directories(FCComponentGallery PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

SET_BIN_DIR(FCComponentGallery FCComponentGallery)

install(TARGETS FCComponentGallery DESTINATION ${CMAKE_INSTALL_BINDIR})
