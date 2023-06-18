cmake_minimum_required(VERSION 3.24)
project(audio C CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(audio main.cpp AudioFile.cpp AudioFile.h)
