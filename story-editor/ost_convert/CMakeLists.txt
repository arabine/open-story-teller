cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME ost-convert)
project(${PROJECT_NAME} LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#=========================================================================================================================
# INCLUDES
#=========================================================================================================================
set(OST_CONVERT_ROOT  ${CMAKE_CURRENT_SOURCE_DIR})
set(OST_ROOT  ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(OST_CONVERT_INCLUDES
    ${OST_ROOT}/common
)

#=========================================================================================================================
# SOURCES
#=========================================================================================================================


#=========================================================================================================================
# TARGET
#=========================================================================================================================

add_executable(${PROJECT_NAME} main.cpp ni_parser.c pack_archive.cpp zip.cpp miniz.c command_line.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${OST_CONVERT_INCLUDES})
