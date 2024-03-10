# Story Editor

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
