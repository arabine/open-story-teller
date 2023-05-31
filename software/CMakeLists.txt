# ==================================================================================================
# OPEN STORY TELLER MAIN CMAKE BUILD FILE
# (see README.md for invokable options)
# Default target: TBD ?
# ==================================================================================================

cmake_minimum_required(VERSION 3.13)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE ON)
set(PROJECT_NAME open-story-teller)

# ==================================================================================================
# DEFAULT VARIABLES OPTIONS
# ==================================================================================================
if(NOT OST_BUNDLE)
    set(OST_BUNDLE "RASPI_PICO")
endif()

if(NOT PICO_SDK_PATH)
    set(PICO_SDK_PATH "../pico-sdk")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ==================================================================================================
# OST COMMON FILES
# ==================================================================================================
set(OST_SRCS
    system/main.c
    system/sdcard.c
    system/debug.c
    system/picture.c
    system/filesystem.c
    system/ost_tasker.c
    system/ff/ff.c
    system/ff/ffsystem.c
    system/ff/ff_stubs.c
)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/library
    ${CMAKE_CURRENT_SOURCE_DIR}/system
    ${CMAKE_CURRENT_SOURCE_DIR}/system/ff
)

# ==================================================================================================
# BUNDLE LONGAN NANO
# ==================================================================================================
if(${OST_BUNDLE} STREQUAL "LONGAN_NANO")
    message("=============  TARGET: LONGAN NANO  =============")

    add_definitions(-DOST_USE_FF_LIBRARY)

    # ------ CPU
    SET(CPU_TARGET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cpu/gigadevice-gd32vf103)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cpu/gigadevice-gd32vf103/include.cmake)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cpu/gigadevice-gd32vf103)

    # ------ PLATFORM
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/platform/sipeed-longan-nano)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/platform/sipeed-longan-nano)

    # ------ MIDDLEWARE
    target_sources(${PROJECT_NAME} PRIVATE library/ili9341.c)

    # link with gcc for software library such as float calculation (__udivdi3 ...)
    add_library(BUNDLE_LIBRARIES INTERFACE)
    target_link_libraries(BUNDLE_LIBRARIES INTERFACE "-Wl,--start-group"
        gcc m c gigadevice-gd32vf103 sipeed-longan-nano
        "-Wl,--end-group")

# ==================================================================================================
# BUNDLE MKR ZERO
# ==================================================================================================
elseif(${OST_BUNDLE} STREQUAL "MKR_ZERO")
    message("=============  TARGET: MKR ZERO  =============")

    add_definitions(-DOST_USE_FF_LIBRARY)

    # ------ CPU
    SET(CPU_TARGET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cpu/atmel-samd21)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cpu/atmel-samd21/include.cmake)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cpu/atmel-samd21)

    # ------ PLATFORM
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/platform/arduino-mkr-zero)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/platform/arduino-mkr-zero)

    # ------ MIDDLEWARE
    target_sources(${PROJECT_NAME} PRIVATE library/st7789.c)

    add_library(BUNDLE_LIBRARIES INTERFACE)
    target_link_libraries(BUNDLE_LIBRARIES INTERFACE "-Wl,--start-group"
        gcc m c stdc++ atmel-samd21 arduino-mkr-zero "-Wl,--end-group")

    project(${PROJECT_NAME} LANGUAGES CXX C ASM)

# target_link_directories(BUNDLE_LIBRARIES INTERFACE ${CMAKE_SOURCE_DIR}/libs/CMSIS/CMSIS/Lib/GCC)

# ==================================================================================================
# BUNDLE RASPBERRY PICO
# ==================================================================================================
elseif(${OST_BUNDLE} STREQUAL "RASPI_PICO")
    message("=============  TARGET: RASPBERRY PICO  =============")

    add_definitions(-DOST_USE_FF_LIBRARY)

    # initialize the SDK based on PICO_SDK_PATH
    # note: this must happen before project()
    include(${CMAKE_CURRENT_SOURCE_DIR}/platform/raspberry-pico-w/pico_sdk_import.cmake)

    # add_library(BUNDLE_LIBRARIES INTERFACE)
    # target_link_libraries(BUNDLE_LIBRARIES INTERFACE )
    project(${PROJECT_NAME} LANGUAGES CXX C ASM)

    # initialize the Raspberry Pi Pico SDK
    # after project!
    pico_sdk_init()

    # ------ PLATFORM
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/platform/raspberry-pico-w)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/platform/raspberry-pico-w)

    # ------ MIDDLEWARE
    set(MIDDLEWARE_SRCS library/st7789.c)

    add_executable(${PROJECT_NAME} ${OST_SRCS} ${MIDDLEWARE_SRCS})

    # ------ Link
    target_link_libraries(${PROJECT_NAME} PUBLIC "-Wl,--start-group"
        gcc m c stdc++ raspberry-pico-w pico_stdlib hardware_spi hardware_dma "-Wl,--end-group")
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")

    pico_add_extra_outputs(${PROJECT_NAME})
else()
    message("============= DEFAULT TARGET: LINUX CONSOLE ============= ")
endif()

# ==================================================================================================
# CUSTOM TARGETS
# ==================================================================================================
add_custom_target(RaspberryPico
    COMMAND ${CMAKE_COMMAND}
    -DOST_BUNDLE=RASPI_PICO
    -DPICO_SDK_PATH=../pico-sdk
    -DPICO_BOARD=pico_w
    -DCMAKE_BUILD_TYPE=Debug
    -B "${CMAKE_CURRENT_BINARY_DIR}/RaspberryPico"
    -S "${CMAKE_SOURCE_DIR}"
    COMMAND ${CMAKE_COMMAND} --build "${CMAKE_CURRENT_BINARY_DIR}/RaspberryPico"
)
