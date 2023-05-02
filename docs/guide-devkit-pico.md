# Development kit: Raspberry Pico

This is the official development kit. It offers the following advantages:
1. Low cost
2. Parts are easy to find
3. Low wiring/soldering: usage of a motherboard 
4. SWD debug port

Current status: ON DEVELOPMENT

![pico](./images/devkit-pico.jpg)


## Bill of materials

The minimal list of components are:

| Part                                      | Price    | Shop      |
| ----------------------------------------- | -------- | --------- |
| Audio board + speaker                     | 13 €     | Waveshare |
| Raspberry Pico W                          | 9 €      | Kubii     |
| 2inch LCD  (320x240)                      | 14 €     | Waveshare |
| Some Pimoroni buttons are rotary switches | 4 €      | Pimoroni  |
| UPS module or Pimoroni LiPo Shim          | 15 €     | Waveshare |
| LiPo battery 500mAh                       | 9 €      | Any       |
| Carte d'extension GPIO Pico Decker        | 15 €     | Waveshare |
| **TOTAL**                                 | **67 €** |

In addition to this list, you may need some more materials such as wires, prototype boards, resistors...

We may propose in the future a PCB to help the connection without soldering.

![pico](./images/prototype-board.png)


# Developers: how to build from the source code

## Install build tools

Install build tools, example for a Debian based operating system:

- sudo apt install gcc-arm-none-eabi
- sudo apt install picolibc-arm-none-eabi

Download the pico SDK somewhere on your disk:

```
git clone https://github.com/raspberrypi/pico-sdk
```



Copy past the following command line, execute at the directory root. Replace the PICO_SDK_PATH value with the real location on your disk where you have installed the Pico SDK.

First, create a CMake build directory:

```
mkdir build
cd build
```
Then generate the makefile (we use the Pico toolchain here, so there is no specific toolchain file to setup.)

```
cmake  -DCMAKE_BUILD_TYPE=Debug -DOST_BUNDLE=RASPI_PICO -DPICO_SDK_PATH=../pico-sdk -DPICO_BOARD=pico_w ..
```

This assume that the Pico SDK is located on the git project root directory. Change this path according to your real Pico SDK location.





