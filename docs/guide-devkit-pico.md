# Development kit: Raspberry Pico

This is the official development kit. It offers the following advantages:
1. Low cost
2. Parts are easy to find
3. Low wiring/soldering: usage of a motherboard 
4. SWD debug port

Current status: ON DEVELOPMENT

![pico](./images/devkit-pico.jpg)


## Bill of materials

Here is an example of components.
- Links are examples of what to buy, price is not optimized
- Use traditional suppliers for generic components (buttons, resistors...), such as Adafruit, PiHut, Pimoroni, Kubii, Gotronic...

| Part                                  | Price    | Shop      | Link                                     |
| ------------------------------------- | -------- | --------- | ---------------------------------------- |
| Audio board + speaker                 | 13 €     | Waveshare | [Buy on Amazon](https://amzn.to/41nWgeB) |
| Raspberry Pico W                      | 9 €      | Kubii     | [Buy on Amazon](https://amzn.to/3AUQeXQ) |
| 2inch LCD  (320x240)                  | 14 €     | Waveshare | [Buy on Amazon](https://amzn.to/3LyG5oJ) |
| Some push buttons and rotary switches | 4 €      | Any       | [Buy on Amazon](https://amzn.to/3AX6MOX) |
| UPS module or Pimoroni LiPo Shim      | 15 €     | Waveshare | [Buy on Amazon](https://amzn.to/44p8Exo) |
| LiPo battery 500mAh                   | 9 €      | Any       | [Buy on Amazon](https://amzn.to/3VCl3df) |
| Carte d'extension GPIO Pico 4 modules | 15 €     | Waveshare | [Buy on Amazon](https://amzn.to/42ukJQ4) |
| **TOTAL**                             | **67 €** |

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





