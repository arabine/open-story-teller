# ==================================================================================================
# GigaDevice GD32VF103
# ==================================================================================================

cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME gigadevice-gd32vf103)
project(${PROJECT_NAME} LANGUAGES CXX C ASM)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GD32VF103_SRCS
    systick.c
    RISCV/drivers/n200_func.c
    RISCV/env_Eclipse/handlers.c
    RISCV/env_Eclipse/init.c
    RISCV/env_Eclipse/start.S
    RISCV/env_Eclipse/entry.S
    RISCV/env_Eclipse/your_printf.c
    RISCV/stubs/_exit.c
    RISCV/stubs/close.c
    RISCV/stubs/isatty.c
    RISCV/stubs/lseek.c
    RISCV/stubs/read.c
    RISCV/stubs/sbrk.c
    RISCV/stubs/write_hex.c
    RISCV/stubs/write.c

    # Standard peripheral library
    GD32VF103_standard_peripheral/system_gd32vf103.c
    GD32VF103_standard_peripheral/Source/gd32vf103_dma.c
    GD32VF103_standard_peripheral/Source/gd32vf103_eclic.c
    GD32VF103_standard_peripheral/Source/gd32vf103_gpio.c
    GD32VF103_standard_peripheral/Source/gd32vf103_spi.c
    GD32VF103_standard_peripheral/Source/gd32vf103_timer.c
    GD32VF103_standard_peripheral/Source/gd32vf103_usart.c
    GD32VF103_standard_peripheral/Source/gd32vf103_rcu.c
)

include_directories(../../src)

add_library(
    ${PROJECT_NAME}
    STATIC 
    ${GD32VF103_SRCS}
)

