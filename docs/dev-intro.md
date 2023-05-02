# Software development

## Firmware/software

The firmware is highly configurable and highly portable. To achieve that, it is split in multiple parts:
- The core source file, which is common to every target
- the ports, dedicated to an embedded MCU and board
- The tests, to easily test part of source on a standard PC
- A desktop/mobile implementation

The core is written in pure C, targets implementations may add other languages and libraries (QML/C++/python ...).


## CMake build system

The project uses CMake as build system. For the embedded targets, the main CMakeLists.txt includes a generic cross compiler file that should be good for many configurations as soon as your compiler is GCC.

## Invocation

1. Create a build directory (mkdir build)
2. Invoke cmake with some options, passed as definitions for CMake (-Doption)

| Option               | Role                                                      |
| -------------------- | --------------------------------------------------------- |
| TOOLCHAIN            | specify the prefix name of the cross GCC binary           |
| CMAKE_TOOLCHAIN_FILE | Includes before everything else a compiler toolchain file |
| CMAKE_BUILD_TYPE     | Debug or Release (default ?)                              |
| OST_BUNDLE           | Specify the bundle name to build                          |
| TOOLCHAIN_DIR        | Specify a directory for the cross-gcc toolchain location  |


Example: `cmake -DTOOLCHAIN=riscv64-unknown-elf -DCMAKE_TOOLCHAIN_FILE=cmake/cross-gcc.cmake  -DCMAKE_BUILD_TYPE=Debug -DOST_BUNDLE=LONGAN_NANO ..`

















