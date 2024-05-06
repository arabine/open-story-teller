# ===========================================================================
# CMAKE PRINCIPAL APPELÉ PAR GRADLE
# ===========================================================================

cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

project(main C CXX)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

include(FetchContent)


#=========================================================================================================================
# APPLI
#=========================================================================================================================
include(../story.cmake)

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
FetchContent_MakeAvailable(sdl_image)
include_directories(${sdl_image_SOURCE_DIR}/include)


#=========================================================================================================================
# TARGET
#=========================================================================================================================
add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})


target_include_directories(${PROJECT_NAME} PUBLIC ${sdl3_SOURCE_DIR}/include ${PROJECT_INCLUDES})
target_link_directories(${PROJECT_NAME} PUBLIC  ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} ${sdl3_BINARY_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC dl GLESv1_CM GLESv2 OpenSLES SDL3 SDL3_image::SDL3_image android log)

target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra -Wdocumentation -Wdocumentation-unknown-command -Wmissing-prototypes -Wunreachable-code-break -Wunneeded-internal-declaration -Wmissing-variable-declarations 
 -Wfloat-conversion -Wshorten-64-to-32 -Wunreachable-code-return -Wshift-sign-overflow -Wstrict-prototypes -Wkeyword-macro -Wno-unused-parameter -Wno-sign-compare)


