

# ==================================================================================================
# 
# ==================================================================================================

cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME atmel-samd21)
project(${PROJECT_NAME} LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SAMD21_SRCS
    startup_samd21.c
    system_samd21.c
    spi_master.c
    spi_display.c
    uart.c
)

include_directories(../../src)

add_library(
    ${PROJECT_NAME}
    STATIC 
    ${SAMD21_SRCS}
)

