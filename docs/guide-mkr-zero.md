## Arduino MKR Zero (SAMD21G18A)

| Category           | Maker                          | Name                  | Rounded Price |
| ------------------ | ------------------------------ | --------------------- | ------------- |
| Main CPU board     | Arduino                        | MKR Zero              | 30€           |
| Audio              |                                |                       | 15€           |
| Memory storage     | Included SD card slot on board |                       | -             |
| Battery management | Included LiPo charger on board |                       | -             |
| Display            | NewHaven  2.4" TFT             | NHD-2.4-240320CF-BSXV | 22€           |

### How to build

Install on Ubuntu : 
- sudo apt install gcc-arm-none-eabi
- sudo apt install picolibc-arm-none-eabi

cmake -DTOOLCHAIN=arm-none-eabi -DCMAKE_TOOLCHAIN_FILE=cmake/cross-gcc.cmake  -DCMAKE_BUILD_TYPE=Debug -DOST_BUNDLE=MKR_ZERO ..

### Wiring

TBD