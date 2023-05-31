/*
The MIT License

Copyright (c) 2022 Anthony Rabine
Copyright (c) 2018 Mario Falcao

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef CHIP32_H
#define CHIP32_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>


// General form:  instruction destination, source
//     coded as:   instr   dst, src
typedef enum
{
    // system:
    OP_NOP = 0,  ///< do nothing
    OP_HALT = 1, ///< halt execution
    OP_SYSCALL = 2,  ///< system call handled by user-registered function, 4 arguments (R0 - R3) passed by value
    // constants:
    OP_LCONS = 3,  ///< store a value in a register, e.g.: lcons r0, 0xA2 0x00 0x00 0x00
                   ///< can also load a variable address: lcons r2, $DataInRam

    // register operations:
    OP_MOV = 4, ///< copy a value between registers, e.g.: mov r0, r2
    // stack:
    OP_PUSH = 5, ///< push a register onto the stack, e.g.: push r0
    OP_POP = 6,  ///< pop the first element of the stack to a register, e.g.: pop r0

    // memory get/set from/to an address. The last argument is a size (1, 2 or 4 bytes)
    OP_STORE = 7,   ///< copy a value from a register to a ram address located in a register, specific size e.g. : store @r4, r1, 2 (2 bytes)
    OP_LOAD = 8,   ///< copy a value from a ram address located in a register to a register, specific size e.g.: load r0, @r3, 1 (1 byte)

    // arithmetic:
    OP_ADD = 9,  ///<  sum and store in first reg, e.g.: add r0, r2
    OP_SUB = 10,  ///<  subtract and store in first reg, e.g.: sub r0, r2
    OP_MUL = 11,  ///<  multiply and store in first reg, e.g.: mul r0, r2
    OP_DIV = 12,  ///<  divide and store in first reg, remain in second, e.g.: div r0, r2

    OP_SHL = 13,  ///<  logical shift left, e.g.: shl r0, r1
    OP_SHR = 14,  ///<  logical shift right, e.g.: shr r0, r1
    OP_ISHR = 15, ///<  arithmetic shift right (for signed values), e.g.: ishr r0, r1

    OP_AND = 16,  ///<  and two registers and store result in the first one, e.g.: and r0, r1
    OP_OR = 17,   ///<  or two registers and store result in the first one, e.g.: or r0, r1
    OP_XOR = 18,  ///<  xor two registers and store result in the first one, e.g.: xor r0, r1
    OP_NOT = 19,  ///<  not a register and store result, e.g.: not r0

    // branching/functions
    OP_CALL = 20, ///<  set register RA to the next instruction and jump to subroutine, e.g.: call 0x10 0x00
    OP_RET = 21,  ///<  return to the address of last callee (RA), e.g.: ret
    OP_JUMP = 22, ///<   jump to address (can use label or address), e.g.: jump .my_label
    OP_JUMPR = 23, ///<   jump to address contained in a register, e.g.: jumpr t9
    OP_SKIPZ = 24,  ///<  skip next instruction if zero, e.g.: skipz r0
    OP_SKIPNZ = 25, ///<  skip next instruction if not zero, e.g.: skipnz r2

    INSTRUCTION_COUNT
} chip32_instruction_t;


/*

| name  | number | type                             | preserved |
|-------|--------|----------------------------------|-----------|
| r0-r9 | 0-9    | general-purpose                  | Y         |
| t0-t9 | 10-19  | temporary registers              | N         |
| pc    | 20     | program counter                  | Y         |
| sp    | 21     | stack pointer                    | Y         |
| ra    | 22     | return address                   | N         |

*/
typedef enum
{
    // preserved across a call, use them for argument passing
    R0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    R8,
    R9,
    // Temporaries are automatically saved on stack across a call
    T0,
    T1,
    T2,
    T3,
    T4,
    T5,
    T6,
    T7,
    T8,
    T9,
    // special
    PC,
    SP,
    RA,
    // count
    REGISTER_COUNT
} chip32_register_t;


typedef enum
{
    VM_FINISHED,                // execution completed (i.e. got halt instruction)
    VM_SKIPED,                  // skipped instruction
    VM_WAIT_EVENT,              // execution paused since we hit the maximum instructions
    VM_OK,                      // execution ok
    VM_ERR_UNKNOWN_OPCODE,      // unknown opcode
    VM_ERR_UNSUPPORTED_OPCODE,  // instruction not supported on this platform
    VM_ERR_INVALID_REGISTER,    // invalid register access
    VM_ERR_UNHANDLED_INTERRUPT, // interrupt triggered without registered handler
    VM_ERR_STACK_OVERFLOW,      // stack overflow
    VM_ERR_STACK_UNDERFLOW,     // stack underflow
    VM_ERR_INVALID_ADDRESS,     // tried to access an invalid memory address
} chip32_result_t;

typedef struct {
    uint8_t opcode;
    uint8_t nbAargs; //!< Number of arguments needed in assembly
    uint8_t bytes; //!< Size of bytes arguments
} OpCode;

#define OPCODES_LIST { { OP_NOP, 0, 0 }, { OP_HALT, 0, 0 }, { OP_SYSCALL, 1, 1 }, { OP_LCONS, 2, 5 }, \
{ OP_MOV, 2, 2 }, { OP_PUSH, 1, 1 }, {OP_POP, 1, 1 }, \
{ OP_STORE, 3, 4 }, { OP_LOAD, 3, 4 }, { OP_ADD, 2, 2 }, { OP_SUB, 2, 2 }, { OP_MUL, 2, 2 }, \
{ OP_DIV, 2, 2 }, { OP_SHL, 2, 2 }, { OP_SHR, 2, 2 }, { OP_ISHR, 2, 2 }, { OP_AND, 2, 2 }, \
{ OP_OR, 2, 2 }, { OP_XOR, 2, 2 }, { OP_NOT, 1, 1 }, { OP_CALL, 1, 1 }, { OP_RET, 0, 0 }, \
{ OP_JUMP, 1, 2 }, { OP_JUMPR, 1, 1 }, { OP_SKIPZ, 1, 1 }, { OP_SKIPNZ, 1, 1 } }

/**
  Whole memory is 64KB

  The RAM and ROM segments can be placed anywhere:
   - they must not overlap
   - there can have gaps between segments (R/W to a dummy byte if accessed)

 -----------  0xFFFF
 |         |
 |         |
 |---------|<-- stack start
 |         |
 |   RAM   |
 |_________|
 |         |
 |_________|
 |  ROM    |
 |_________|
 |         |
 -----------  0x0000
 */
typedef struct
{
    uint8_t *mem; //!< Pointer to a real memory location (ROM or RAM)
    uint16_t size; //!< Size of the real memory
    uint16_t addr; //!< Start address of the virtual memory

} virtual_mem_t;

typedef struct chip32_ctx_t chip32_ctx_t;

typedef uint8_t (*syscall_t)(chip32_ctx_t *, uint8_t);

#define SYSCALL_RET_OK          0   ///< Default state, continue execution immediately
#define SYSCALL_RET_WAIT_EV     1   ///< Sets the VM in wait for event state


struct chip32_ctx_t
{
    virtual_mem_t rom;
    virtual_mem_t ram;
    uint16_t stack_size;
    uint32_t instrCount;
    uint16_t prog_size;
    uint32_t max_instr;
    uint32_t registers[REGISTER_COUNT];
    syscall_t syscall;

};

// =======================================================================================
// VM RUN
// =======================================================================================
void chip32_initialize(chip32_ctx_t *ctx);
chip32_result_t chip32_run(chip32_ctx_t *ctx); // loop until the end or max instructions
chip32_result_t chip32_step(chip32_ctx_t *ctx); // one instruction


#ifdef __cplusplus
}
#endif

#endif // CHIP32_H
