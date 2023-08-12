cmake_minimum_required(VERSION 3.11)

project(story-editor VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets Multimedia OpenGL)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Gui OpenGLWidgets Multimedia OpenGL )

set(PROJECT_SOURCES
    src/main.cpp
    src/main_window.h
    src/main_window.cpp
    src/toolbar.h
    src/toolbar.cpp
    src/ost-editor.qrc
    src/story_project.cpp
    src/story_project.h
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
    src/osthmi_dock.h
    src/osthmi_dock.cpp
    src/log_dock.h
    src/log_dock.cpp
    src/vm_dock.h
    src/vm_dock.cpp
    src/new_project_dialog.h
    src/new_project_dialog.cpp
    src/dock_widget_base.h
    src/dock_widget_base.cpp
    src/code_editor.h
    src/code_editor.cpp
    src/graph.h
    src/graph.cpp
    src/event_node_model.h
    src/event_node_model.cpp
    src/highlighter.h
    src/highlighter.cpp
    src/dr_mp3.h
    src/media_converter.h
    src/media_converter.cpp
    src/ost-hmi.ui
    src/ost-vm.ui
    src/ost-data.ui
    src/ost-script.ui
    src/ost-resources.ui
    src/ost-log.ui
    src/about.ui
    src/media-node.ui
    src/event-node.ui
    src/choose-file.ui
    src/new-project.ui
    ../software/chip32/chip32_assembler.cpp
    ../software/chip32/chip32_vm.c
    ../software/library/miniaudio.c
    ../software/library/miniaudio.h
)

include_directories(../software/chip32)
include_directories(../software/library)

add_subdirectory(QHexView)

set(CMAKE_AUTOMOC ON)

set(NODEEDITOR_SOURCE_FILES
  ./nodeeditor/src/AbstractGraphModel.cpp
  ./nodeeditor/src/AbstractNodeGeometry.cpp
  ./nodeeditor/src/BasicGraphicsScene.cpp
  ./nodeeditor/src/ConnectionGraphicsObject.cpp
  ./nodeeditor/src/ConnectionPainter.cpp
  ./nodeeditor/src/ConnectionState.cpp
  ./nodeeditor/src/ConnectionStyle.cpp
  ./nodeeditor/src/DataFlowGraphModel.cpp
  ./nodeeditor/src/DataFlowGraphicsScene.cpp
  ./nodeeditor/src/DefaultHorizontalNodeGeometry.cpp
  ./nodeeditor/src/DefaultVerticalNodeGeometry.cpp
  ./nodeeditor/src/Definitions.cpp
  ./nodeeditor/src/GraphicsView.cpp
  ./nodeeditor/src/GraphicsViewStyle.cpp
  ./nodeeditor/src/NodeDelegateModelRegistry.cpp
  ./nodeeditor/src/NodeConnectionInteraction.cpp
  ./nodeeditor/src/NodeDelegateModel.cpp
  ./nodeeditor/src/NodeGraphicsObject.cpp
  ./nodeeditor/src/DefaultNodePainter.cpp
  ./nodeeditor/src/NodeState.cpp
  ./nodeeditor/src/NodeStyle.cpp
  ./nodeeditor/src/StyleCollection.cpp
  ./nodeeditor/src/UndoCommands.cpp
  ./nodeeditor/src/locateNode.cpp
  ./nodeeditor/resources/resources.qrc
)

set(NODEEDITOR_HEADER_FILES
  ./nodeeditor/include/QtNodes/internal/AbstractGraphModel.hpp
  ./nodeeditor/include/QtNodes/internal/AbstractNodeGeometry.hpp
  ./nodeeditor/include/QtNodes/internal/AbstractNodePainter.hpp
  ./nodeeditor/include/QtNodes/internal/BasicGraphicsScene.hpp
  ./nodeeditor/include/QtNodes/internal/Compiler.hpp
  ./nodeeditor/include/QtNodes/internal/ConnectionGraphicsObject.hpp
  ./nodeeditor/include/QtNodes/internal/ConnectionIdHash.hpp
  ./nodeeditor/include/QtNodes/internal/ConnectionIdUtils.hpp
  ./nodeeditor/include/QtNodes/internal/ConnectionState.hpp
  ./nodeeditor/include/QtNodes/internal/ConnectionStyle.hpp
  ./nodeeditor/include/QtNodes/internal/DataFlowGraphicsScene.hpp
  ./nodeeditor/include/QtNodes/internal/DataFlowGraphModel.hpp
  ./nodeeditor/include/QtNodes/internal/DefaultNodePainter.hpp
  ./nodeeditor/include/QtNodes/internal/Definitions.hpp
  ./nodeeditor/include/QtNodes/internal/Export.hpp
  ./nodeeditor/include/QtNodes/internal/GraphicsView.hpp
  ./nodeeditor/include/QtNodes/internal/GraphicsViewStyle.hpp
  ./nodeeditor/include/QtNodes/internal/locateNode.hpp
  ./nodeeditor/include/QtNodes/internal/NodeData.hpp
  ./nodeeditor/include/QtNodes/internal/NodeDelegateModel.hpp
  ./nodeeditor/include/QtNodes/internal/NodeDelegateModelRegistry.hpp
  ./nodeeditor/include/QtNodes/internal/NodeGraphicsObject.hpp
  ./nodeeditor/include/QtNodes/internal/NodeState.hpp
  ./nodeeditor/include/QtNodes/internal/NodeStyle.hpp
  ./nodeeditor/include/QtNodes/internal/OperatingSystem.hpp
  ./nodeeditor/include/QtNodes/internal/QStringStdHash.hpp
  ./nodeeditor/include/QtNodes/internal/QUuidStdHash.hpp
  ./nodeeditor/include/QtNodes/internal/Serializable.hpp
  ./nodeeditor/include/QtNodes/internal/Style.hpp
  ./nodeeditor/include/QtNodes/internal/StyleCollection.hpp
  ./nodeeditor/include/QtNodes/internal/ConnectionPainter.hpp
  ./nodeeditor/include/QtNodes/internal/DefaultHorizontalNodeGeometry.hpp
  ./nodeeditor/include/QtNodes/internal/DefaultVerticalNodeGeometry.hpp
  ./nodeeditor/include/QtNodes/internal/NodeConnectionInteraction.hpp
  ./nodeeditor/include/QtNodes/internal/UndoCommands.hpp
)

if (WIN32)
    list(APPEND PROJECT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/icon.rc")
endif()

qt_add_executable(${PROJECT_NAME}
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    ${NODEEDITOR_SOURCE_FILES}
    ${NODEEDITOR_HEADER_FILES}
)

target_compile_definitions(${PROJECT_NAME} PUBLIC NODE_EDITOR_STATIC)
target_include_directories(${PROJECT_NAME} PUBLIC nodeeditor/include nodeeditor/include/QtNodes/internal)

target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::Multimedia
    QHexView
    )




if (UNIX)
    target_link_libraries(${PROJECT_NAME} PUBLIC dl)
endif (UNIX)

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER storyeditor.d8s.eu
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
