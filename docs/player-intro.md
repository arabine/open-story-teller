---
outline: deep
---
# Software player

The Story Player is a minimal implementation of OpenStoryTeller in puerely software (without hardware device)

![player](images/story_player.png)

Technolologies used:
- C language
- Raylib for graphics and sounds
- CMake build system

It should be possible to run it everywhere where Raylib can run.

# How to use

Just open the C32 Virtual Machine binary file ("story.c32", typically), images and sounds must follow standard project organisation.

## How to build

The source code is available in the `story-player` sub-directory. From the command line:

```
mkdir build
cmake ..
make
```

## TODO

1. Drag and drop zipped story archive
2. Stories library manager


