cmake_minimum_required(VERSION 3.5)

set(STORY_PLAYER_PROJECT story-player)

project(${STORY_PLAYER_PROJECT} LANGUAGES C CXX)

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


include(FetchContent)

# =========================================================================================================================
# SDL3
# =========================================================================================================================
FetchContent_Declare(
    sdl3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG 78cc5c173404488d80751af226d1eaf67033bcc4   # === prerelease-3.1.0
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

set(BUILD_SHARED_LIBS TRUE)
set(SDL_STATIC TRUE)
FetchContent_MakeAvailable(sdl3)
include_directories(${sdl3_SOURCE_DIR}/include)
# add_subdirectory(${sdl3_SOURCE_DIR}) 

# =========================================================================================================================
# SDL3-Image
# =========================================================================================================================
FetchContent_Declare(
    sdl_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG origin/main
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

set(SDL3IMAGE_INSTALL OFF)
set(BUILD_SHARED_LIBS FALSE)
FetchContent_MakeAvailable(sdl_image)
include_directories(${sdl_image_SOURCE_DIR}/include)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
include(./story.cmake)

if(WIN32)
    list(APPEND
        PROJECT_SOURCES
        icon.rc
    )
endif()

if(WIN32)
    add_executable(${STORY_PLAYER_PROJECT}
        WIN32
        ${PROJECT_SOURCES}
    )
else()
    add_executable(${STORY_PLAYER_PROJECT}
        ${PROJECT_SOURCES}

    )
endif()

target_include_directories(${STORY_PLAYER_PROJECT} PUBLIC
    ${sdl3_SOURCE_DIR}/include

    ${PROJECT_INCLUDES}

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

target_compile_definitions(${STORY_PLAYER_PROJECT} PUBLIC cimg_display=0)

target_compile_definitions(${STORY_PLAYER_PROJECT} PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")

target_link_directories(${STORY_PLAYER_PROJECT} PUBLIC ${sdl3_BINARY_DIR} ${sdl_image_BINARY_DIR})

# On est obligé de passer par une variable pour injecter 
# certaines informations à CPACK
set(SDL_BIN_DIR ${sdl3_BINARY_DIR})

if(UNIX)
    target_link_libraries(${STORY_PLAYER_PROJECT}
        pthread
        OpenGL::GL
        dl
        SDL3::SDL3
        SDL3_image::SDL3_image

     #   OpenSSL::SSL OpenSSL::Crypto
    )
elseif(WIN32)
    target_link_libraries(${STORY_PLAYER_PROJECT}
        OpenGL::GL
        SDL3::SDL3
        SDL3_image::SDL3_image
        libcurl_static
        ws2_32.lib psapi.lib setupapi.lib cfgmgr32.lib advapi32.lib
    )
endif()

# =========================================================================================================================
# CPACK INSTALLER
# =========================================================================================================================
install(TARGETS ${STORY_PLAYER_PROJECT} RUNTIME DESTINATION ".")

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
    install_files("." FILES "${SDL_BIN_DIR}/SDL3.dll")
    install_files("." FILES "/usr/lib/gcc/x86_64-w64-mingw32/10-posix/libstdc++-6.dll")
    install_files("." FILES "/usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll")
    install_files("." FILES "/usr/lib/gcc/x86_64-w64-mingw32/10-posix/libgcc_s_seh-1.dll")

    # Personnaliser l'icône pour les installateurs Windows
    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/story-editor-logo.ico") 
endif()

if (APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/bundle.plist.in)
    install_files("." FILES "${SDL_BIN_DIR}/libSDL2-2.0.0.dylib")
endif()

include(CPack)
