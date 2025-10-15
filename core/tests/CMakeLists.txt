cmake_minimum_required(VERSION 3.11)

project(core_tests LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================
# Téléchargement et configuration de Catch2 v3
# ============================================================================
include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.7.1  # Dernière version stable de Catch2 v3
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(Catch2)

# Ajouter le module Catch2 pour la découverte automatique des tests
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

# ============================================================================
# Exécutable de tests
# ============================================================================
add_executable(${PROJECT_NAME} 
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
    ../chip32/chip32_binary_format.c
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ../chip32  
    ../story-manager/src
    ../story-manager/src/nodes
    ../story-manager/src/compiler
    ../story-manager/interfaces
    ../../shared
)

# ============================================================================
# Linkage avec Catch2
# ============================================================================
target_link_libraries(${PROJECT_NAME} PRIVATE Catch2::Catch2WithMain)

# ============================================================================
# Découverte automatique des tests (optionnel mais recommandé)
# ============================================================================
include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME})