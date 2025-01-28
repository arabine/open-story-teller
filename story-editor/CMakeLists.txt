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

set(IMGUI_VERSION 1.91.6)

include(FetchContent)
# Adhere to GNU filesystem layout conventions
include(GNUInstallDirs)


# =========================================================================================================================
# IMGUI and plugins
# =========================================================================================================================
FetchContent_Declare(
    imgui
    URL https://github.com/ocornut/imgui/archive/refs/tags/v${IMGUI_VERSION}-docking.zip
)

FetchContent_MakeAvailable(imgui)

# ImGuiFileDialog
include_directories(${imgui_SOURCE_DIR})
add_compile_definitions(CUSTOM_IMGUIFILEDIALOG_CONFIG="${CMAKE_SOURCE_DIR}/src/CustomImGuiFileDialogConfig.h")
add_compile_definitions(IMGUI_INCLUDE="imgui.h")
add_subdirectory(libs/ImGuiFileDialog)


# =========================================================================================================================
# SDL3 MIXER
# =========================================================================================================================

add_subdirectory(externals/civetweb EXCLUDE_FROM_ALL)
add_subdirectory(externals/curl EXCLUDE_FROM_ALL)

# Configure SDL by calling its CMake file.
# we use EXCLUDE_FROM_ALL so that its install targets and configs don't
# pollute upwards into our configuration.
add_subdirectory(externals/SDL EXCLUDE_FROM_ALL)

# SDL_mixer (used for playing audio)
set(SDLMIXER_MIDI_NATIVE OFF)     # disable formats we don't use to make the build faster and smaller. Also some of these don't work on all platforms so you'll need to do some experimentation.
set(SDLMIXER_GME OFF)
set(SDLMIXER_WAVPACK OFF)     
set(SDLMIXER_MOD OFF)
set(SDLMIXER_OPUS OFF)
set(SDLMIXER_VENDORED ON)   # tell SDL_mixer to build its own dependencies
add_subdirectory(externals/SDL_mixer EXCLUDE_FROM_ALL)

# SDL_image (used for loading various image formats)
set(SDLIMAGE_VENDORED OFF)
set(SDLIMAGE_AVIF OFF)	# disable formats we don't use to make the build faster and smaller.
set(SDLIMAGE_BMP ON)
set(SDLIMAGE_JPEG ON)
set(SDLIMAGE_WEBP ON)
add_subdirectory(externals/SDL_image EXCLUDE_FROM_ALL)


# =========================================================================================================================
# Project sources
# =========================================================================================================================
set(SRCS

    src/main.cpp

    src/windows/window_base.cpp
    src/windows/console_window.cpp
    src/windows/emulator_window.cpp
    src/windows/main_window.cpp
    src/windows/library_window.cpp
    src/windows/resources_window.cpp
    src/windows/properties_window.cpp
    src/windows/debugger_window.cpp
    src/windows/cpu_window.cpp
    src/windows/variables_window.cpp
    
    src/node_editor/media_node_widget.cpp
    src/node_editor/base_node_widget.cpp
    src/node_editor/node_editor_window.cpp
    src/node_editor/function_node_widget.cpp
    src/node_editor/variable_node_widget.cpp
   
    src/gui.cpp
    src/media_converter.cpp
    src/web_server.cpp

    src/importers/pack_archive.cpp
    src/importers/ni_parser.c

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

    ../shared/audio_player.cpp
    ../shared/stb_vorbis.c
    ../shared/resource_manager.cpp
    ../shared/library_manager.cpp
    ../shared/downloader.cpp
    ../shared/story_db.cpp
    ../shared/miniz.c
    ../shared/zip.cpp
    ../shared/platform_folders.cpp
    ../shared/raudio.c


    # Core engine files
    ../core/story-manager/src/compiler.cpp
    ../core/story-manager/src/story_project.cpp
    ../core/story-manager/src/story_page.cpp
    ../core/story-manager/src/base_node.cpp
    ../core/story-manager/src/media_node.cpp
    ../core/story-manager/src/compare_node.cpp
    ../core/story-manager/src/branch_node.cpp
    ../core/story-manager/src/variable_node.cpp
    ../core/story-manager/src/function_node.cpp
    ../core/story-manager/src/connection.cpp
    ../core/story-manager/lib/sys_lib.cpp
    ../core/story-manager/lib/resource.cpp


    ../core/chip32/chip32_assembler.cpp
    ../core/chip32/chip32_vm.c
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
    ${imgui_SOURCE_DIR}/backends

    libs/ImGuiFileDialog
    libs/imgui-node-editor
    libs

    src
    src/importers
    src/node_editor
    src/windows

    ../firmware/library
    ../core/chip32
    ../shared
    ../core/story-manager/src
    ../core/story-manager/lib
    ../core/story-manager/interfaces
)

add_definitions(
    -DIMGUI_USE_WCHAR32 
    -DVERSION_MAJOR=${PROJECT_VERSION_MAJOR} 
    -DVERSION_MINOR=${PROJECT_VERSION_MINOR} 
    -DVERSION_PATCH=${PROJECT_VERSION_PATCH}

    -DRAUDIO_STANDALONE
    -DSUPPORT_MODULE_RAUDIO
)
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


target_link_directories(${STORY_EDITOR_PROJECT} PUBLIC 
    ${sdl3_BINARY_DIR}
    ${libcurl_BINARY_DIR}
    ${mbedtls_BINARY_DIR}
)

# On est obligé de passer par une variable pour injecter 
# certaines informations à CPACK
set(SDL_BIN_DIR ${sdl3_BINARY_DIR})
set(SDL_IMAGE_BIN_DIR ${sdl_image_BINARY_DIR})
set(SDL_MIXER_BIN_DIR ${sdl3_mixer_BINARY_DIR})

if(UNIX)
    target_link_libraries(${STORY_EDITOR_PROJECT}
        SDL3_image::SDL3_image
        SDL3_mixer::SDL3_mixer
        SDL3::SDL3
        CURL::libcurl
        civetweb::civetweb-cpp
        pthread
        OpenGL::GL
        dl

    )
elseif(WIN32)
    target_link_libraries(${STORY_EDITOR_PROJECT}
        OpenGL::GL
        SDL3_image::SDL3_image
        SDL3_mixer::SDL3_mixer
        SDL3::SDL3
        libcurl_static
        ws2_32.lib psapi.lib setupapi.lib cfgmgr32.lib advapi32.lib
    )
endif()

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/OUTPUT" CACHE PATH "Directory for sbnw installation" FORCE)

# set_target_properties(${STORY_EDITOR_PROJECT}
#     PROPERTIES
#         LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/lib
#         RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/bin
# )


# =========================================================================================================================
# DIRECTORY INSTALLER
# =========================================================================================================================

# install(DIRECTORY "${PROJECT_SOURCE_DIR}/assets/" DESTINATION "assets")
install(DIRECTORY "${PROJECT_SOURCE_DIR}/fonts/" DESTINATION "fonts")
install(DIRECTORY "${PROJECT_SOURCE_DIR}/scripts/" DESTINATION "scripts")
install(FILES "${CMAKE_SOURCE_DIR}/LICENSE" DESTINATION ".")
install(FILES "${CMAKE_SOURCE_DIR}/tools/imgui.ini" DESTINATION "bin")
install(TARGETS ${STORY_EDITOR_PROJECT} BUNDLE DESTINATION bin)

if(WIN32)
    install_files("." FILES "${SDL_BIN_DIR}/SDL3.dll")
    install_files("." FILES "${SDL_IMAGE_BIN_DIR}/SDL3_image.dll")
    install_files("." FILES "${SDL_MIXER_BIN_DIR}/SDL3_mixer.dll")
    install_files("." FILES "/usr/lib/gcc/x86_64-w64-mingw32/10-posix/libstdc++-6.dll")
    install_files("." FILES "/usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll")
    install_files("." FILES "/usr/lib/gcc/x86_64-w64-mingw32/10-posix/libgcc_s_seh-1.dll")

    # Personnaliser l'icône pour les installateurs Windows
    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/story-editor-logo.ico") 
endif()

if(LINUX)

    install(DIRECTORY "${SDL_BIN_DIR}/" DESTINATION lib FILES_MATCHING REGEX "libSDL3.so(\\..*)?$")
    install(DIRECTORY "${SDL_IMAGE_BIN_DIR}/" DESTINATION lib FILES_MATCHING REGEX "libSDL3_image.so(\\..*)?$")
    install(DIRECTORY "${SDL_MIXER_BIN_DIR}/" DESTINATION lib FILES_MATCHING REGEX "libSDL3_mixer.so(\\..*)?$")

    # install_files("." FILES "${SDL_BIN_DIR}/libSDL3.so")
    # install_files("." FILES "${SDL_IMAGE_BIN_DIR}/libSDL3_image.so")
    # install_files("." FILES "${SDL_MIXER_BIN_DIR}/libSDL3_mixer.so")
endif()

if (APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/bundle.plist.in)
    install_files("." FILES "${SDL_BIN_DIR}/libSDL2-2.0.0.dylib")
endif()

