cmake_minimum_required(VERSION 3.5)

set(STORY_EDITOR_PROJECT story-editor)

project(${STORY_EDITOR_PROJECT} LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Version du projet
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)

# set(CMAKE_VERBOSE_MAKEFILE on)
if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()

find_package(OpenGL REQUIRED)

find_package(OpenSSL REQUIRED)

set(IMGUI_VERSION 1.90)

include(FetchContent)

# =========================================================================================================================
# CURL
# =========================================================================================================================
FetchContent_Declare(curl
    URL https://github.com/curl/curl/archive/refs/tags/curl-8_6_0.zip
)


set(BUILD_CURL_EXE FALSE)
set(BUILD_STATIC_LIBS TRUE)
FetchContent_MakeAvailable(curl)

# =========================================================================================================================
# IMGUI and plugins
# =========================================================================================================================
FetchContent_Declare(imgui
    URL https://github.com/ocornut/imgui/archive/refs/tags/v${IMGUI_VERSION}-docking.zip
)

FetchContent_GetProperties(imgui)

if(NOT imgui_POPULATED)
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(imgui)
endif()

# ImGuiFileDialog
include_directories(${imgui_SOURCE_DIR})
add_compile_definitions(CUSTOM_IMGUIFILEDIALOG_CONFIG="${CMAKE_SOURCE_DIR}/src/CustomImGuiFileDialogConfig.h")
add_compile_definitions(IMGUI_INCLUDE="imgui.h")
add_subdirectory(libs/ImGuiFileDialog)

# =========================================================================================================================
# SDL2
# =========================================================================================================================

# FetchContent_Declare(
#     sdl2
#     GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
#     GIT_TAG origin/SDL2
#     GIT_SHALLOW TRUE
#     GIT_PROGRESS TRUE
# )

# set(BUILD_SHARED_LIBS TRUE)
# set(SDL_STATIC TRUE)
# FetchContent_MakeAvailable(sdl2)

# =========================================================================================================================
# SDL3
# =========================================================================================================================
FetchContent_Declare(
    sdl3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG prerelease-3.1.0
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

set(BUILD_SHARED_LIBS TRUE)
set(SDL_STATIC TRUE)
FetchContent_MakeAvailable(sdl3)
include_directories(${sdl3_SOURCE_DIR}/include)

# =========================================================================================================================
# SDL3-Image
# =========================================================================================================================
# FetchContent_Declare(
# SDL2_image
# GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
# GIT_TAG origin/main
# GIT_SHALLOW TRUE
# GIT_PROGRESS TRUE
# )

# # START ADDITION
# set(SDL2IMAGE_INSTALL OFF)
# set(BUILD_SHARED_LIBS FALSE)
# # END ADDITION

add_subdirectory(libs/SDL_image) 
include_directories(libs/SDL_image/include)

# FetchContent_MakeAvailable(SDL2_image)
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

    src/library_window.cpp
    src/library_window.h

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

    src/media_converter.cpp
    src/media_converter.h

    src/i_story_manager.h

    libs/ImGuiColorTextEdit/TextEditor.cpp
    libs/ImGuiColorTextEdit/TextEditor.h
    libs/imgui-node-editor/imgui_node_editor.cpp
    libs/imgui-node-editor/imgui_canvas.cpp
    libs/imgui-node-editor/imgui_node_editor_api.cpp
    libs/imgui-node-editor/crude_json.cpp
    libs/ImGuiFileDialog/ImGuiFileDialog.cpp

    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp

    ../software/chip32/chip32_assembler.cpp
    ../software/chip32/chip32_vm.c

    ../software/common/audio_player.cpp
    ../software/common/audio_player.h
    ../software/common/tlv.h

    ../software/library/miniaudio.h
    ../software/library/uuid.h
    ../software/library/resource.h
    ../software/library/resource_manager.h
    ../software/library/story_project.cpp
    ../software/library/story_project.h
    ../software/library/thread_safe_queue.h
    ../software/library/library_manager.h
    ../software/library/library_manager.cpp
)

if(WIN32)
    list(APPEND
        SRCS
        icon.rc
    )
endif()

if(WIN32)
    add_executable(${STORY_EDITOR_PROJECT}
        WIN32
        ${SRCS}
    )
else()
    add_executable(${STORY_EDITOR_PROJECT}
        ${SRCS}

    )
endif()

target_include_directories(${STORY_EDITOR_PROJECT} PUBLIC
    ${imgui_SOURCE_DIR}
    ${sdl2_SOURCE_DIR}/include
    libs/ImGuiColorTextEdit/
    ${imgui_SOURCE_DIR}/backends
    libs/ImGuiFileDialog
    libs/imgui-node-editor

    ${curl_INCLUDE_DIR}

    ../software/library/
    ../software/chip32/
    ../software/common
)

add_definitions(-DIMGUI_USE_WCHAR32 -DVERSION_MAJOR=${PROJECT_VERSION_MAJOR} -DVERSION_MINOR=${PROJECT_VERSION_MINOR} -DVERSION_PATCH=${PROJECT_VERSION_PATCH})
add_link_options(-static-libgcc -static-libstdc++)

if (APPLE)
set_target_properties(${PROJECT_NAME} PROPERTIES
        LINKER_LANGUAGE CXX
        LINK_FLAGS "-Wl,-rpath,@executable_path"
        MACOSX_RPATH TRUE
        SKIP_BUILD_RPATH FALSE
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH_USE_LINK_PATH TRUE
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    BUNDLE True
    MACOSX_BUNDLE_GUI_IDENTIFIER eu.d8s.OpenStoryTeller.StoryEditor.${CUR_TARGET}
    MACOSX_BUNDLE_BUNDLE_NAME ${CUR_TARGET}
    MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/bundle.plist.in
)

endif()

target_compile_definitions(${STORY_EDITOR_PROJECT} PUBLIC cimg_display=0)

target_compile_definitions(${STORY_EDITOR_PROJECT} PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")

target_link_directories(${STORY_EDITOR_PROJECT} PUBLIC ${sdl3_BINARY_DIR} ${curl_BINARY_DIR} ${CMAKE_BINARY_DIR}/libs/SDL_image)


set(SDL2_BIN_DIR ${sdl3_BINARY_DIR})

if(UNIX)
    target_link_libraries(${STORY_EDITOR_PROJECT}
        pthread
        OpenGL::GL
        dl
        SDL3
        SDL3_image
        libcurl_static
        OpenSSL::SSL OpenSSL::Crypto
    )
elseif(WIN32)
    target_link_libraries(${STORY_EDITOR_PROJECT}
        OpenGL::GL
        SDL2
        ws2_32.lib psapi.lib setupapi.lib cfgmgr32.lib advapi32.lib
    )
endif()

# =========================================================================================================================
# CPACK INSTALLER
# =========================================================================================================================
install(TARGETS ${STORY_EDITOR_PROJECT} RUNTIME DESTINATION ".")

# Personnaliser les options d'installation
set(CPACK_PACKAGE_NAME "Open-Story-Editor")
set(CPACK_PACKAGE_DESCRIPTION "Open Story Teller - Node based editor")
set(CPACK_PACKAGE_VENDOR "D8S")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

# install(DIRECTORY "${PROJECT_SOURCE_DIR}/assets/" DESTINATION "assets")
install(DIRECTORY "${PROJECT_SOURCE_DIR}/fonts/" DESTINATION "fonts")
install(DIRECTORY "${PROJECT_SOURCE_DIR}/scripts/" DESTINATION "scripts")
install_files("." FILES "${CMAKE_SOURCE_DIR}/LICENSE")
install_files("." FILES "${CMAKE_SOURCE_DIR}/tools/imgui.ini")

if(WIN32)
    install_files("." FILES "${SDL2_BIN_DIR}/SDL2.dll")
    install_files("." FILES "/usr/lib/gcc/x86_64-w64-mingw32/10-posix/libstdc++-6.dll")
    install_files("." FILES "/usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll")
    install_files("." FILES "/usr/lib/gcc/x86_64-w64-mingw32/10-posix/libgcc_s_seh-1.dll")
endif()

# Personnaliser l'icône pour les installateurs Windows
if(WIN32)
    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/story-editor-logo.ico")
endif()

if (APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/bundle.plist.in)
    install_files("." FILES "${SDL2_BIN_DIR}/libSDL2-2.0.0.dylib")
endif()

include(CPack)
