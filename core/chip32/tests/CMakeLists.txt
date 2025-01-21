cmake_minimum_required(VERSION 3.5)

project(chip32_test LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(chip32_test 
    main.cpp 
    test_parser.cpp 
    test_vm.cpp 
    ../chip32_assembler.cpp 
    ../chip32_vm.c
)

target_include_directories(chip32_test PRIVATE ../)

install(TARGETS chip32_test
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
