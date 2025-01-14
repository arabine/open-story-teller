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

# set(OPENSSL_ROOT_DIR /libs/openssl)
# find_package(OpenSSL REQUIRED)

set(IMGUI_VERSION 1.91.6)

include(FetchContent)

include(cmake/CPM.cmake)


# =========================================================================================================================
# MBedTLS
# =========================================================================================================================
CPMAddPackage(
    NAME mbedtls    
    GITHUB_REPOSITORY Mbed-TLS/mbedtls
    VERSION 3.6.2
    OPTIONS
        "USE_STATIC_MBEDTLS_LIBRARY ON"
        "ENABLE_PROGRAMS OFF"
        "ENABLE_TESTING OFF"
)

find_package(MbedTLS REQUIRED)
include_directories(${mbedtls_INCLUDE_DIR})
# set(MBEDTLS_STATIC_LIBRARY ON)

# =========================================================================================================================
# CibetWeb
# =========================================================================================================================
CPMAddPackage(
    NAME civetweb
    GITHUB_REPOSITORY  civetweb/civetweb
    VERSION 1.16
    OPTIONS 
        "CIVETWEB_BUILD_TESTING OFF"
        "CIVETWEB_ENABLE_SERVER_EXECUTABLE OFF"
        "CIVETWEB_ENABLE_CXX ON"
        "CIVETWEB_ENABLE_WEBSOCKETS ON"
        "CIVETWEB_ENABLE_ASAN OFF"
)
 
find_package(civetweb REQUIRED)
include_directories(${civetweb_SOURCE_DIR}/include)

# =========================================================================================================================
# CURL
# =========================================================================================================================


# Définit les options de cURL pour utiliser mBedTLS

set(CURL_USE_OPENSSL OFF  CACHE BOOL "Disable OpenSSL." FORCE)
set(CURL_USE_MBEDTLS ON  CACHE BOOL "Use MBED TLS." FORCE)
set(CURL_USE_LIBSSH2 OFF  CACHE BOOL "Disable SSH." FORCE)
set(MBEDTLS_INCLUDE_DIRS ${mbedtls_SOURCE_DIR}/include)
set(MBEDTLS_LIBRARY ${mbedtls_BINARY_DIR}/libmbedtls.a)
set(MBEDX509_LIBRARY ${mbedtls_BINARY_DIR}/libmbedx509.a)
set(MBEDCRYPTO_LIBRARY ${mbedtls_BINARY_DIR}/libmbedcrypto.a)

# Télécharge et configure cURL
FetchContent_Declare(
  curl
  GIT_REPOSITORY https://github.com/curl/curl.git
  GIT_TAG        curl-8_7_1
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE
)

set(BUILD_CURL_EXE FALSE)
set(BUILD_STATIC_LIBS TRUE)
FetchContent_MakeAvailable(curl)

# Assurez-vous que votre projet trouve les headers de mBedTLS et cURL
include_directories(${mbedtls_SOURCE_DIR}/include)
include_directories(${curl_SOURCE_DIR}/include)


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
# SDL3
# =========================================================================================================================
FetchContent_Declare(
    sdl3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG 78cc5c173404488d80751af226d1eaf67033bcc4   # === preview-3.1.6
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

set(BUILD_SHARED_LIBS TRUE)
set(SDL_STATIC TRUE)
FetchContent_MakeAvailable(sdl3)
include_directories(${sdl3_SOURCE_DIR}/include)
# add_subdirectory(${sdl3_SOURCE_DIR}) 

# =========================================================================================================================
# SDL3 MIXER
# =========================================================================================================================

CPMAddPackage(
   NAME sdl3_mixer
   URL https://github.com/libsdl-org/SDL_mixer/archive/d4eba31e4ac23a81fffad02e91b17dcb2449a2cb.tar.gz

    OPTIONS 
        "BUILD_SHARED_LIBS TRUE"
        "SDL_PULSEAUDIO_SHARED TRUE"
        "SDL_PIPEWIRE_SHARED TRUE" 
)

# FetchContent_Declare(
#     sdl3_mixer
#     GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
#     GIT_TAG d4eba31e4ac23a81fffad02e91b17dcb2449a2cb
#     # GIT_SHALLOW TRUE # Ne pas activer shallow sinon le tag n'est pas trouvé
#     # GIT_PROGRESS TRUE
#     GIT_SUBMODULES ""
# )

# set(BUILD_SHARED_LIBS TRUE)
# set(SDL_PULSEAUDIO_SHARED TRUE)
# set(SDL_PIPEWIRE_SHARED TRUE)
# FetchContent_MakeAvailable(sdl3_mixer)
# include_directories(${sdl3_mixer_SOURCE_DIR}/include)


# =========================================================================================================================
# SDL3-Image
# =========================================================================================================================
FetchContent_Declare(
    sdl_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG bcc97c044266080256ef6ed0d690859677212b2b
    GIT_PROGRESS TRUE
    # GIT_SHALLOW TRUE # Ne pas activer shallow sinon le tag n'est pas trouvé
) 


set(SDL3IMAGE_INSTALL OFF)
set(BUILD_SHARED_LIBS TRUE)
FetchContent_MakeAvailable(sdl_image)
include_directories(${sdl_image_SOURCE_DIR}/include)

# =========================================================================================================================
# Project sources
# =========================================================================================================================
set(SRCS

    src/main.cpp

    src/window_base.cpp
    src/console_window.cpp
    src/emulator_window.cpp
    src/main_window.cpp
    src/library_window.cpp
    src/platform_folders.cpp


    src/node_editor/media_node_widget.cpp
    src/node_editor/base_node_widget.cpp
    src/node_editor/node_editor_window.cpp
    src/node_editor/function_node_widget.cpp

    src/resources_window.cpp
    src/properties_window.cpp
    src/cpu_window.cpp
    src/gui.cpp

    src/code_editor.cpp
    src/media_converter.cpp
    src/miniz.c
    src/zip.cpp
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

    ../firmware/chip32/chip32_assembler.cpp
    ../firmware/chip32/chip32_vm.c

    ../shared/audio_player.cpp
    ../shared/stb_vorbis.c
    ../shared/resource_manager.cpp
    ../shared/library_manager.cpp
    ../shared/downloader.cpp
    ../shared/story_db.cpp

    # Core engine files
    
    ../core/src/compiler.cpp
    ../core/src/story_project.cpp
    ../core/src/story_page.cpp
    ../core/src/base_node.cpp
    ../core/src/media_node.cpp
    ../core/src/function_node.cpp
    ../core/src/connection.cpp

    ../core/lib/sys_lib.cpp
    ../core/lib/resource.cpp
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

    # ${te_SOURCE_DIR}
    
    ${imgui_SOURCE_DIR}/backends
    libs/ImGuiFileDialog
    libs/imgui-node-editor

    ${curl_INCLUDE_DIR}

    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/importers
    ${CMAKE_SOURCE_DIR}/src/node_editor
    ${CMAKE_SOURCE_DIR}/src/node_engine

    ../firmware/library
    ../firmware/chip32
    ../shared
    ../core/src
    ../core/lib
    ../core/interfaces
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

# target_compile_definitions(${STORY_EDITOR_PROJECT} PUBLIC cimg_display=0)

# target_compile_definitions(${STORY_EDITOR_PROJECT} PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")

target_link_directories(${STORY_EDITOR_PROJECT} PUBLIC 
    ${sdl3_BINARY_DIR}
    ${curl_BINARY_DIR}
    ${mbedtls_BINARY_DIR}
    # ${CivetWeb_BINARY_DIR}
)

# On est obligé de passer par une variable pour injecter 
# certaines informations à CPACK
set(SDL_BIN_DIR ${sdl3_BINARY_DIR})
set(SDL_IMAGE_BIN_DIR ${sdl_image_BINARY_DIR})
set(SDL_MIXER_BIN_DIR ${sdl3_mixer_BINARY_DIR})

if(UNIX)
    target_link_libraries(${STORY_EDITOR_PROJECT}

        
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_mixer::SDL3_mixer
        libcurl_static
        mbedtls
        civetweb-cpp
        pthread
        OpenGL::GL
        dl

    )
elseif(WIN32)
    target_link_libraries(${STORY_EDITOR_PROJECT}
        OpenGL::GL
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_mixer::SDL3_mixer
        libcurl_static
        ws2_32.lib psapi.lib setupapi.lib cfgmgr32.lib advapi32.lib
    )
endif()

# =========================================================================================================================
# CPACK INSTALLER
# =========================================================================================================================
install(TARGETS ${STORY_EDITOR_PROJECT} RUNTIME DESTINATION ".")

# Personnaliser les options d'installation
set(CPACK_PACKAGE_NAME "Open-Story-Editor")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Anthony Rabine")
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

    install_files("." FILES "${SDL_BIN_DIR}/libSDL3.so")
    install_files("." FILES "${SDL_IMAGE_BIN_DIR}/libSDL3_image.so")
    install_files("." FILES "${SDL_MIXER_BIN_DIR}/libSDL3_mixer.so")
endif()

if (APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/bundle.plist.in)
    install_files("." FILES "${SDL_BIN_DIR}/libSDL2-2.0.0.dylib")
endif()

include(CPack)
