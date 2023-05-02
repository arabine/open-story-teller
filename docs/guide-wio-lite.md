## Sipeed Longan Nano (GD32VF103CBT6) - TO BE REPLACED WITH WIO LITE

Current status: ON DEVELOPMENT

## What does it look like

The firmware is still under construction. Everything is tested on breadboard.

![proto](images/ost-1/complete.png)

What is working:
- The audio path
- The SD Card
- Roughly: playing a wav file from the SD Card

## Audio path

An I2S DAC controller with a jack output :

![proto](images/ost-1/audio_board.webp)

An audio amplifier from Adafruit (2.5W, can drive a speaker between 3 ohms and 8 ohms).

![proto](images/ost-1/audio_amplifier.png)

A speaker :

![proto](images/ost-1/speaker_4ohms_3w.png)



| Category           | Maker                                | Name        | Rounded Price |
| ------------------ | ------------------------------------ | ----------- | ------------- |
| Main CPU board     | Sipeed                               | Longan Nano | 4€            |
| Audio              |                                      |             | 15€           |
| Memory storage     | Included SD card slot in Longan Nano |             | -             |
| Battery management |                                      |             | 15€           |


| Part                                              | Price    | Shop       |
| ------------------------------------------------- | -------- | ---------- |
| PCM5102 Audio board                               | 4 €      | Aliexpress |
| PAM8302 Mono Amplifier                            | 9 €      | Adafruit   |
| Longan Nano RISC-V board with SD-Card port        | 4 €      | Aliexpress |
| 3.2" SPI TFT Screen (320x240) with ILI9341 driver | 9 €      | Aliexpress |
| Adafruit PowerBoost 500 charger                   | 15 €     | Adafruit   |
| Some Pimoroni buttons are rotary switches         | 4 €      | Pimoroni   |
| Speaker                                           | 4 €      | Pimoroni   |
| LiPo battery 500mAh                               | 9 €      | Any        |
| **TOTAL**                                         | **58 €** |



### How to build

Tools for a Debian based distro

- sudo apt install crossbuild-essential-riscv64
- sudo apt install picolibc-riscv64-unknown-elf
  
mkdir build
cd build
cmake -DTOOLCHAIN=riscv64-unknown-elf -DCMAKE_TOOLCHAIN_FILE=cmake/cross-gcc.cmake  -DCMAKE_BUILD_TYPE=Debug -DOST_BUNDLE=LONGAN_NANO ..

Convert tools:

- riscv64-unknown-elf-objcopy -O binary your-file.elf your-file.hex
- riscv64-unknown-elf-objcopy -O ihex your-file.elf your-file.hex

### Wiring

TBD
