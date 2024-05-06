cmake_minimum_required(VERSION 3.10)

project(storyvm LANGUAGES CXX C)
add_library(storyvm
  SHARED
  ./storyvm.cpp
  ../../firmware/chip32/chip32_vm.c
)

include_directories(../../firmware/chip32)

