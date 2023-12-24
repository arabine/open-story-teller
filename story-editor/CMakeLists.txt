cmake_minimum_required(VERSION 3.5)

set(STORY_EDITOR_PROJECT story-editor)

project(${STORY_EDITOR_PROJECT} LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_VERBOSE_MAKEFILE on)


if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()
find_package(OpenGL REQUIRED)

set(IMGUI_VERSION 1.90)

include(FetchContent)


#=========================================================================================================================
# IMGUI and plugins
#=========================================================================================================================
FetchContent_Declare(imgui
  URL https://github.com/ocornut/imgui/archive/refs/tags/v${IMGUI_VERSION}-docking.zip
)


FetchContent_GetProperties(imgui)
if (NOT imgui_POPULATED)
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(imgui)
endif()

# ImGuiFileDialog
include_directories(${imgui_SOURCE_DIR})
add_compile_definitions(CUSTOM_IMGUIFILEDIALOG_CONFIG="${CMAKE_SOURCE_DIR}/src/CustomImGuiFileDialogConfig.h")
add_compile_definitions(IMGUI_INCLUDE="imgui.h")
add_subdirectory(libs/ImGuiFileDialog)


#=========================================================================================================================
# SDL
#=========================================================================================================================

Set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
    sdl2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        origin/SDL2
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

set(BUILD_SHARED_LIBS TRUE)
set(SDL_STATIC TRUE)
FetchContent_MakeAvailable(sdl2)
#add_subdirectory(libs/SDL)
#include_directories(libs/SDL/include)


#=========================================================================================================================
# SDL3-Image
#=========================================================================================================================
#FetchContent_Declare(
#        SDL2_image
#        GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
#        GIT_TAG origin/main
#        GIT_SHALLOW TRUE
#        GIT_PROGRESS TRUE
#)

## START ADDITION
#set(SDL2IMAGE_INSTALL OFF)
#set(BUILD_SHARED_LIBS FALSE)
## END ADDITION

#FetchContent_MakeAvailable(SDL2_image)


set(SRCS

    src/main.cpp

    src/window_base.h
    src/window_base.cpp

    src/console_window.cpp
    src/console_window.h

    src/emulator_window.cpp
    src/emulator_window.h

    src/main_window.cpp
    src/main_window.h

    src/node_editor_window.cpp
    src/node_editor_window.h

    src/media_node.h
    src/media_node.cpp

    src/platform_folders.cpp
    src/platform_folders.h

    src/base_node.h
    src/base_node.cpp

    src/resources_window.cpp
    src/resources_window.h

    src/properties_window.cpp
    src/properties_window.h

    src/gui.h
    src/gui.cpp

    src/code_editor.cpp
    src/code_editor.h

    src/connection.cpp
    src/connection.h

    src/uuid.h
    src/resource_manager.h
    src/i_story_project.h
    src/resource.h

    libs/ImGuiColorTextEdit/TextEditor.cpp
    libs/ImGuiColorTextEdit/TextEditor.h
    libs/imgui-node-editor/imgui_node_editor.cpp
    libs/imgui-node-editor/imgui_canvas.cpp
    libs/imgui-node-editor/imgui_node_editor_api.cpp
    libs/imgui-node-editor/crude_json.cpp
    libs/ImGuiFileDialog/ImGuiFileDialog.cpp

    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp


    ../software/chip32/chip32_assembler.cpp
    ../software/chip32/chip32_vm.c
    ../software/library/miniaudio.c
    ../software/library/miniaudio.h

    src/story_project.cpp
    src/story_project.h
    src/media_converter.cpp
    src/media_converter.h
)

if(WIN32)
    list(APPEND
        SRCS
        icon.rc
    )
endif()

add_executable(${STORY_EDITOR_PROJECT}
    ${SRCS}
)

target_include_directories(${STORY_EDITOR_PROJECT} PUBLIC
    ${imgui_SOURCE_DIR}
    ${sdl2_SOURCE_DIR}/include
    libs/ImGuiColorTextEdit/
    ${imgui_SOURCE_DIR}/backends
    libs/ImGuiFileDialog
    libs/imgui-node-editor

    ../software/library/
    ../software/chip32/
)



add_definitions(-DIMGUI_USE_WCHAR32)
add_link_options(-static-libgcc -static-libstdc++)

target_compile_definitions(${STORY_EDITOR_PROJECT} PUBLIC cimg_display=0)

target_compile_definitions(${STORY_EDITOR_PROJECT} PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")

target_link_directories(${STORY_EDITOR_PROJECT} PUBLIC ${sdl2_BINARY_DIR})
message(${sdl2_BINARY_DIR})
if (UNIX)
target_link_libraries(${STORY_EDITOR_PROJECT}
    pthread
    udev
    OpenGL::GL
    dl
    SDL2
)
elseif(WIN32)

target_compile_features("-Wl,-subsystem,windows")

target_link_libraries(${STORY_EDITOR_PROJECT}
    OpenGL::GL
#    SDL2::SDL2main
    SDL2::SDL2-static
    ws2_32.lib psapi.lib setupapi.lib cfgmgr32.lib advapi32.lib Dbghelp.lib
)
#set_target_properties(${STORY_EDITOR_PROJECT} PROPERTIES
#LINK_FLAGS /SUBSYSTEM:CONSOLE
#)
endif()

