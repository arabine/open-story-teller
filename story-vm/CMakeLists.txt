cmake_minimum_required(VERSION 3.10)

project(storyvm LANGUAGES CXX C)
add_executable(storyvm
  
  ./storyvm.cpp
  ../firmware/chip32/chip32_vm.c
)

include_directories(../firmware/chip32 ../shared)

# Spécifier les options de compilation pour Emscripten
set_target_properties(storyvm PROPERTIES
    COMPILE_FLAGS "-s WASM=1"
    LINK_FLAGS "-s WASM=1 -s EXPORTED_FUNCTIONS='[\"_storyvm_start\", \"_storyvm_send_event\"]' -s EXPORTED_RUNTIME_METHODS='[\"ccall\", \"cwrap\"]'"
)
