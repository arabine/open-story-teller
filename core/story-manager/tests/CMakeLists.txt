cmake_minimum_required(VERSION 3.5)

project(compiler_test LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} 
    main.cpp 
    test_ast.cpp 


    ../src/nodes/base_node.cpp
    ../src/nodes/branch_node.cpp
    ../src/nodes/print_node.cpp
    ../src/nodes/variable_node.cpp
    ../src/nodes/syscall_node.cpp
    ../src/nodes/connection.cpp

    ../../chip32/chip32_assembler.cpp 
    ../../chip32/chip32_vm.c
)

target_include_directories(${PROJECT_NAME}  PRIVATE 
    ../../chip32  
    ../src 
    ../interfaces 
    ../lib 
    ../../../shared
    ../src/nodes
    ../src/compiler
)
