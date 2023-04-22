cmake_minimum_required(VERSION 3.11)

project(story-editor VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets OpenGLWidgets OpenGL)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Gui OpenGL OpenGLWidgets)

set(PROJECT_SOURCES
    src/main.cpp
    src/main_window.h
    src/main_window.cpp
    src/toolbar.h
    src/toolbar.cpp
    src/ost-editor.qrc
    src/story_project.cpp
    src/media_node_model.h
    src/media_node_model.cpp
    src/story_graph_model.cpp
    src/story_graph_model.h
    src/story_node_base.cpp
    src/story_node_base.h
    src/story_graph_scene.h
    src/story_graph_scene.cpp
    src/resource_model.h
    src/resource_model.cpp
    src/resources_dock.h
    src/resources_dock.cpp
    src/script_editor_dock.h
    src/script_editor_dock.cpp
    src/memory_view_dock.h
    src/memory_view_dock.cpp
    src/nodeeditor_dock.h
    src/nodeeditor_dock.cpp
    src/osthmi_dock.h
    src/osthmi_dock.cpp
    src/vm_dock.h
    src/vm_dock.cpp
    src/code_editor.h
    src/code_editor.cpp
    src/graph.h
    src/graph.cpp
    src/event_node_model.h
    src/event_node_model.cpp
    src/highlighter.h
    src/highlighter.cpp
    src/ost-hmi.ui
    src/ost-vm.ui
    src/ost-data.ui
    src/ost-script.ui
    src/ost-resources.ui
    src/ost-log.ui
    src/media-node.ui
    src/event-node.ui
    ../software/chip32/chip32_assembler.cpp
    ../software/chip32/chip32_vm.c
)

include_directories(../software/chip32)

add_subdirectory(QHexView)
option(BUILD_SHARED_LIBS "" OFF)
add_subdirectory(nodeeditor)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ost-editor APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(${PROJECT_NAME} SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(${PROJECT_NAME}
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${NODE_EDITOR_HEADERS})
target_link_directories(${PROJECT_NAME} PUBLIC ${NODE_EDITOR_LIBDIR})
target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    Qt${QT_VERSION_MAJOR}::OpenGL
    QtNodes
    QHexView
    )

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()
