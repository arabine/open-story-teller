# ==================================================================================================
# Board Raspberry Pico (W)
# ==================================================================================================

cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME raspberry-pico-w)

project(${PROJECT_NAME} LANGUAGES CXX C ASM)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PICO_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/pico_hal_wrapper.c
    ${CMAKE_CURRENT_LIST_DIR}/pico_lcd_spi.c
    ${CMAKE_CURRENT_LIST_DIR}/pico_sdcard_spi.c
    ${CMAKE_CURRENT_LIST_DIR}/pico_i2s.c
)

include_directories(../../src ../../hal ../../library .)

add_library(
    ${PROJECT_NAME}
    INTERFACE
)

pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/pico_i2s.pio)
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/i2s.pio)

target_link_libraries(${PROJECT_NAME} INTERFACE pico_stdlib hardware_exception cmsis_core)
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_sources(${PROJECT_NAME} INTERFACE
    ${PICO_SRCS}
)
