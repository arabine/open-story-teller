
add_definitions(-DUSE_STDPERIPH_DRIVER)
add_definitions(-DHXTAL_VALUE=8000000U)
add_definitions(-DF_CPU=48000000)

# Object build options
# -O0                   No optimizations, reduce compilation time and make debugging produce the expected results.
# -mthumb               Generat thumb instructions.
# -fno-builtin          Do not use built-in functions provided by GCC.
# -Wall                 Print only standard warnings, for all use Wextra
# -ffunction-sections   Place each function item into its own section in the output file.
# -fdata-sections       Place each data item into its own section in the output file.
# -fomit-frame-pointer  Omit the frame pointer in functions that don’t need one.
# -mabi=aapcs           Defines enums to be a variable sized type.
# set(OBJECT_GEN_FLAGS "-O0 -march=rv32imac -mabi=ilp32 -mcmodel=medlow -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -nostdlib -specs=picolibc.specs")
set(OBJECT_GEN_FLAGS "-march=rv32imac -mabi=ilp32 -mcmodel=medlow -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -specs=picolibc.specs")

# -Wl,--gc-sections     Perform the dead code elimination.
# --specs=nano.specs    Link with newlib-nano.
# --specs=picolibc.specs Link with Pico libC
# --specs=nosys.specs   No syscalls, provide empty implementations for the POSIX system calls.
set(CMAKE_EXE_LINKER_FLAGS "-T${CPU_TARGET_DIR}/RISCV/env_Eclipse/GD32VF103xB.lds -Wl,--no-relax -Wl,--gc-sections -Wl,-M=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map -nostartfiles" CACHE INTERNAL "Linker options")

#---------------------------------------------------------------------------------------
# Set compiler/linker flags
#---------------------------------------------------------------------------------------

set(CMAKE_C_FLAGS   "${OBJECT_GEN_FLAGS} -std=gnu11 -MMD -MP -MF" CACHE INTERNAL "C Compiler options")
set(CMAKE_CXX_FLAGS "${OBJECT_GEN_FLAGS} -std=c++11 " CACHE INTERNAL "C++ Compiler options")
set(CMAKE_ASM_FLAGS "${OBJECT_GEN_FLAGS} -x assembler-with-cpp " CACHE INTERNAL "ASM Compiler options")

include_directories (
    ${CPU_TARGET_DIR}
    ${CPU_TARGET_DIR}/RISCV/drivers
    ${CPU_TARGET_DIR}/RISCV/env_Eclipse
    ${CPU_TARGET_DIR}/RISCV/stubs
    ${CPU_TARGET_DIR}/GD32VF103_standard_peripheral
    ${CPU_TARGET_DIR}/GD32VF103_standard_peripheral/Include
)

