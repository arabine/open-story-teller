cmake_minimum_required(VERSION 3.5)

project(chip32_test LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(chip32_test main.cpp test_parser.cpp test_vm.cpp ../../chip32/chip32_assembler.cpp ../../chip32/chip32.cpp)
target_include_directories(chip32_test PRIVATE ../../chip32 ../)

install(TARGETS chip32_test
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
