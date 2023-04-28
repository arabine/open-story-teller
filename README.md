# Open Story Teller (OST)

Open Story Teller is an Open Source project to provide guidelines and software to build your own story teller box (electronics, mechanical).

The main goal is to *not* make electronics boards but instead buy or reuse existing ones. This will allow you to easily repair your device with spare parts.

We propose a set of parts and firmware that is working well togather but your are free to custom everything to fit your needs.

This project can be used as a base platform for any device that is composed by:
- A display (TFT...)
- An Audio output
- A SD card or memory
- Some Buttons / rotary encoder / potentiometer

# Links

- http://openstoryteller.org/: main documentation and project news
- http://github.com/arabine/open-story-teller: source code, tickets, help

# Generic player software

The base software is highly portable and includes a micro virtual machine. This allow potential complex stories with loops, branches, user choices, randomization...

This project propose an minimal cross-platform GUI player application that can be used as a base project.

# StoryTeller Editor

We propose a basic editor tool to create your own stories. The generated story script runs on our micro virtual machine and allow generate complex stories.

![editor](images/story_editor_preview.png)

Work in progress:
- Project management
- Story pack generation
- Basic nodes (media nodes, start and stop)

Planned nodes:
- Random
- Loop
- Conditional

# License

MIT License

Copyright (c) 2023 Anthony Rabine

