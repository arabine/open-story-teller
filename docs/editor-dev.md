---
outline: deep
---

# Story Editor development

## Source code location

The source code is available in the OpenStory Teller mono-repository [Github](https://github.com/arabine/open-story-teller).

The source code is found in the *story-editor* sub-directory. 

## Development tools

You'll need:

- A C++ compiler
- CMake build utility

Here is a list of packages for Ubuntu-like systems:

```
sudo apt install cmake mesa-utils mesa-common-dev ninja-build libxext-dev libpipewire-0.3-dev libasound2-dev libpulse-dev
```

## How to build

Open the CMakeLists.txt with your favorite IDE (ie, QtCreator, Visual Studio Code) or build from the command line.

## How to generate a Windows executable and setup executable on Ubuntu

The build system uses a Docker environment image for reproductible builds.

Run `build_win32.sh` script.

Output file is located here: `story-editor/build-win32/Open-Story-Editor-1.0.0-win64.exe`

## Linux build

```
cd story-editor
mkdir build
cd build
cmake ..
make -j4
make package
```


# Architecture

![arch](./images/story-editor-architecture.png)






