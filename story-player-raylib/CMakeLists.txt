cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(story-player)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/CPM.cmake)

# =========================================================================================================================
# RAYLIB
# =========================================================================================================================
set(RAYLIB_VERSION 5.5.0)
CPMAddPackage(
    NAME raylib
    URL https://github.com/raysan5/raylib/archive/5.5.tar.gz
    OPTIONS 
        "BUILD_EXAMPLES OFF"
        "BUILD_GAMES OFF"
        "GLFW_BUILD_WAYLAND ON"
        "GLFW_BUILD_X11 ON"
)


# =========================================================================================================================
# EXECUTABLE
# =========================================================================================================================

add_executable(${PROJECT_NAME}
    main.c
    ../firmware/chip32/chip32_assembler.cpp
    ../firmware/chip32/chip32_vm.c
)
include_directories(../firmware/chip32)
include_directories(../firmware/library)

#set(raylib_VERBOSE 1)
target_link_libraries(${PROJECT_NAME} raylib)


# Checks if OSX and links appropriate frameworks (Only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()


if (${PLATFORM} STREQUAL "Web")
    # Tell Emscripten to build an .html file.
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Os")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASYNCIFY=1 -s ASSERTIONS=1 -s WASM=1 -Os -Wall -s TOTAL_MEMORY=67108864 -s FORCE_FILESYSTEM=1 --preload-file ../assets/")
    set_target_properties(${PROJECT_NAME} PROPERTIES 
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../bin"
    )

    target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="/assets/") # Set the asset path macro in release mode to a relative path that assumes the assets folder is in the same directory as the game executable
endif()

