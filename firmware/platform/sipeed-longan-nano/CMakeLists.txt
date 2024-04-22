# ==================================================================================================
# Board Sipeed Longan Nano
# ==================================================================================================

cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME sipeed-longan-nano)
project(${PROJECT_NAME} LANGUAGES CXX C ASM)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LONGAN_NANO_SRCS
    audio.c
    i2s.c
    spi1.c
    spi0.c
    ost_hal_wrapper.c
)

include_directories(../../src ../../hal ../../library)

add_library(
    ${PROJECT_NAME}
    STATIC 
    ${LONGAN_NANO_SRCS}
)

