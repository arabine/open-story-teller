# Open Story Teller (OST)

[![continous_build](https://github.com/arabine/open-story-teller/actions/workflows/build.yml/badge.svg)](https://github.com/arabine/open-story-teller/actions/workflows/build.yml)

Open Story Teller is an Open Source project to provide guidelines and software to build your own story teller box (electronics, mechanical).

The main goal is to *not* make electronics boards but instead buy or reuse existing ones. This will allow you to easily repair your device with spare parts.

We propose a set of parts and firmware that is working well together but your are free to custom everything to fit your needs.

This project can be used as a base platform for any device that is composed by:
- A display (TFT...)
- An Audio output
- A SD card or memory
- Some Buttons / rotary encoder / potentiometer

<hr >

# Links

- http://openstoryteller.org/: main documentation, project news, guidelines, hardware shoping list...
- http://github.com/arabine/open-story-teller: source code, tickets, help

# Hardware parts

We propose the concept of bundles: a set of electronic boards that you can purchase quite easily (eg: Arduino, SeedStudio, Pimoroni, Adafruit, SparkFun...). The bundles are built around a central main board, the CPU, which comes with more or less peripherals (SD-Card reader, Wifi...).

Several bundles will be officially supported ; this is needed for us to ensure the portability of the firmware hardware interface layer.

Current bundles:
- Raspberry Pico (official DevKit) (ARM Cortex M0+)
- RISC-V based board (GD32VF103)

We plan to propose a mechanical enclosure ready to be printed for an official bundle.

See the documentation!

# Generic player architecture

The base software is highly portable and includes a micro virtual machine that executes the story scenario. This allow potential complex stories with loops, branches, user choices, randomization...

This project propose an minimal cross-platform GUI player application that can be used as a base project and VM implementation reference.

# StoryTeller Editor

We propose a basic editor tool to create your own stories. The generated story script runs on our micro virtual machine and allow generate complex stories.

![editor](art/story_editor_preview.png)

Work in progress:
- Project management
- Story pack generation
- Basic nodes (media nodes, start and stop)

Planned nodes:
- Random
- Loop
- Conditional

# Story Player

The Story plater is a purely software implementation of a simple story player. It is provided as an example and a test device for the micro virtual machine. It is very portable and should run on a large number of platforms.

![editor](art/story_player.png)

There are multiple implementations in this directory (Flutter, SDL, Raylib). The one that will be maintain is probably the Raylib version besause this framework is the easiest one to port on multiple platforms.


# Developer corner

## Build the Story Player

### Web version

Emscripten must be installed.

```
mkdir build-web && cd build-web
emcmake cmake .. -DPLATFORM=Web -DCMAKE_BUILD_TYPE=Debug
emmake make
```

# License

MIT License

Copyright (c) 2023 Anthony Rabine

