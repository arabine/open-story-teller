
add_compile_definitions(__SAMD21G18A__)
add_compile_definitions(DONT_USE_CMSIS_INIT)
add_compile_definitions(F_CPU=48000000 )

set(OBJECT_GEN_FLAGS "-O0 -mthumb -fno-builtin -Wall -ffunction-sections -fdata-sections -fomit-frame-pointer -mabi=aapcs -nostdlib")

set(CMAKE_EXE_LINKER_FLAGS "-specs=nosys.specs -nostdlib -mcpu=cortex-m0plus -mthumb -T${CPU_TARGET_DIR}/linker/samd21g18a_flash_without_bootloader.ld -Wl,--no-relax -Wl,--gc-sections -Wl,-M=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map -nostartfiles" CACHE INTERNAL "Linker options")

#---------------------------------------------------------------------------------------
# Set compiler/linker flags
#---------------------------------------------------------------------------------------

set(CMAKE_C_FLAGS   "${OBJECT_GEN_FLAGS} -std=gnu11 -MMD -MP -MF" CACHE INTERNAL "C Compiler options")
set(CMAKE_CXX_FLAGS "${OBJECT_GEN_FLAGS} -std=c++11 -fno-exceptions" CACHE INTERNAL "C++ Compiler options")
set(CMAKE_ASM_FLAGS "${OBJECT_GEN_FLAGS} -x assembler-with-cpp " CACHE INTERNAL "ASM Compiler options")


include_directories (
    ${CPU_TARGET_DIR}
    ${CPU_TARGET_DIR}/3rd-party
    ${CPU_TARGET_DIR}/3rd-party/pio
    ${CPU_TARGET_DIR}/3rd-party/instance
    ${CPU_TARGET_DIR}/3rd-party/component
    ${CPU_TARGET_DIR}/3rd-party/component
    ${CPU_TARGET_DIR}/3rd-party/CMSIS/Include
)

