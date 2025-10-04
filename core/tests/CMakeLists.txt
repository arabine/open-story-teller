cmake_minimum_required(VERSION 3.11)

project(core_tests LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} 
    main.cpp
    test_parser.cpp 
    test_vm.cpp 
    test_ast.cpp
    test_print_node.cpp


    ../story-manager/src/nodes/base_node.cpp
    ../story-manager/src/nodes/branch_node.cpp
    ../story-manager/src/nodes/print_node.cpp
    ../story-manager/src/nodes/variable_node.cpp
    ../story-manager/src/nodes/syscall_node.cpp
    ../story-manager/src/nodes/connection.cpp

    ../chip32/chip32_assembler.cpp 
    ../chip32/chip32_vm.c
)

target_include_directories(${PROJECT_NAME}  PRIVATE 
    ../chip32  
    ../story-manager/src
    ../story-manager/src/nodes
    ../story-manager/src/compiler
    ../story-manager/interfaces
    ../../shared
    
)
