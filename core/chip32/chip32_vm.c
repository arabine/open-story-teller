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

#include "chip32_vm.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// =======================================================================================
// MACROS
// =======================================================================================

#define _NEXT_BYTE ctx->rom.mem[++ctx->registers[PC]]

static inline uint16_t _NEXT_SHORT (chip32_ctx_t *ctx)
{
    ctx->registers[PC] += 2;
    return ctx->rom.mem[ctx->registers[PC]-1] | ctx->rom.mem[ctx->registers[PC]] << 8;
}

static inline uint32_t _NEXT_INT (chip32_ctx_t *ctx)
{
    ctx->registers[PC] += 4;

    return ctx->rom.mem[ctx->registers[PC] - 3] | ctx->rom.mem[ctx->registers[PC] - 2] << 8 |
    ctx->rom.mem[ctx->registers[PC] - 1] << 16 | ctx->rom.mem[ctx->registers[PC]] << 24;
}

#define _CHECK_SKIP if (skip) continue;

#ifndef VM_DISABLE_CHECKS
// Relative address test
#define _CHECK_ROM_ADDR_VALID(a) \
    if (a >= ctx->rom.size) \
        return VM_ERR_INVALID_ADDRESS;
// Relative address test
#define _CHECK_RAM_ADDR_VALID(a) \
    if (a > ctx->ram.size) \
        return VM_ERR_INVALID_ADDRESS;
#define _CHECK_BYTES_AVAIL(n) \
    _CHECK_ROM_ADDR_VALID(ctx->registers[PC] + n)
#define _CHECK_REGISTER_VALID(r) \
    if (r >= REGISTER_COUNT)     \
        return VM_ERR_INVALID_REGISTER;

#define _CHECK_CAN_PUSH(n)                                              \
    if (ctx->registers[SP] - (n * sizeof(uint32_t)) < 0) \
        return VM_ERR_STACK_OVERFLOW;

#define _CHECK_CAN_POP(n)                                               \
    if ((ctx->registers[SP] + (n * sizeof(uint32_t))) > (ctx->ram.size)) \
        return VM_ERR_STACK_UNDERFLOW;
#else
#define _CHECK_ROM_ADDR_VALID(a)
#define _CHECK_BYTES_AVAIL(n)
#define _CHECK_REGISTER_VALID(r)
#define _CHECK_CAN_PUSH(n)
#define _CHECK_CAN_POP(n)
#endif


static const OpCode OpCodes[] = OPCODES_LIST;
static const uint16_t OpCodesSize = sizeof(OpCodes) / sizeof(OpCodes[0]);


static void push(chip32_ctx_t *ctx, uint32_t val) {
    ctx->registers[SP] -= 4;
    ctx->ram.mem[ctx->registers[SP]] = val;
}

static uint32_t pop(chip32_ctx_t *ctx) {
    uint32_t val = ctx->ram.mem[ctx->registers[SP]];
    ctx->registers[SP] += 4;
    return val;
}

// =======================================================================================
// FUNCTIONS
// =======================================================================================
void chip32_initialize(chip32_ctx_t *ctx)
{
    memset(ctx->ram.mem, 0, ctx->ram.size);
    memset(ctx->registers, 0, REGISTER_COUNT * sizeof(uint32_t));
    ctx->instrCount = 0;
    ctx->registers[SP] = ctx->ram.size;
}

chip32_result_t chip32_run(chip32_ctx_t *ctx)
{
    chip32_result_t result = VM_OK;
    while ((ctx->max_instr == 0) || (ctx->instrCount < ctx->max_instr))
    {
        result = chip32_step(ctx);

        if ((result > VM_OK) || (result == VM_FINISHED))
        {
            break;
        }
    }
    return result;
}

chip32_result_t chip32_step(chip32_ctx_t *ctx)
{
    chip32_result_t result = VM_OK;

    _CHECK_ROM_ADDR_VALID(ctx->registers[PC])
    uint8_t instr = ctx->rom.mem[ctx->registers[PC]];
    if (instr >= INSTRUCTION_COUNT)
        return VM_ERR_UNKNOWN_OPCODE;

    uint8_t bytes = OpCodes[instr].bytes;
    _CHECK_BYTES_AVAIL(bytes);

    switch (instr)
    {
    case OP_NOP:
    {
        break;
    }
    case OP_HALT:
    {
        return VM_FINISHED;
    }
    case OP_SYSCALL:
    {
        const uint8_t code = _NEXT_BYTE;

        if (ctx->syscall != NULL)
        {
            if (ctx->syscall(ctx, code) != 0)
            {
                result = VM_WAIT_EVENT;
            }
        }
        break;
    }
    case OP_LCONS:
    {
        const uint8_t reg = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg)
        ctx->registers[reg] = _NEXT_INT(ctx);
        break;
    }
    case OP_MOV:
    {
        const uint8_t reg1 = _NEXT_BYTE;
        const uint8_t reg2 = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg1)
        _CHECK_REGISTER_VALID(reg2)
        ctx->registers[reg1] = ctx->registers[reg2];
        break;
    }
    case OP_PUSH:
    {
        const uint8_t reg = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg)
        _CHECK_CAN_PUSH(1)
        push(ctx, ctx->registers[reg]);
        break;
    }
    case OP_POP:
    {
        const uint8_t reg = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg)
        _CHECK_CAN_POP(1)
        ctx->registers[reg] = pop(ctx);
        break;
    }
    case OP_CALL:
    {
        ctx->registers[RA] = ctx->registers[PC] + 2; // set return address to next instruction after CALL
        const uint8_t reg = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg)
        ctx->registers[PC] = ctx->registers[reg] - 1;

        // Save Tx registers on stack
        _CHECK_CAN_PUSH(10)
        for (int i = 0; i < 10; i++) {
            push(ctx, ctx->registers[T0 + i]);
        }

        break;
    }
    case OP_RET:
    {
        ctx->registers[PC] = ctx->registers[RA] - 1;

        _CHECK_CAN_POP(10)
        // restore Tx registers from stack
        for (int i = 0; i < 10; i++) {
            ctx->registers[T9 - i] = pop(ctx);
        }
        break;
    }
    case OP_STORE:
    {
        const uint8_t reg1 = _NEXT_BYTE;
        const uint8_t reg2 = _NEXT_BYTE;
        const uint8_t size = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg1)
        _CHECK_REGISTER_VALID(reg2)
        // address is located in reg1 reg
        uint32_t addr = ctx->registers[reg1];
        bool isRam = addr & 0x80000000;
        addr &= 0xFFFF; // mask the RAM/ROM bit, ensure 16-bit addressing
        if (isRam) {
            _CHECK_RAM_ADDR_VALID(addr)
            memcpy(&ctx->ram.mem[addr], &ctx->registers[reg2], size);
        } else {
            _CHECK_ROM_ADDR_VALID(addr)
            memcpy(&ctx->rom.mem[addr], &ctx->registers[reg2], size);
        }

        break;
    }
    case OP_LOAD:
    {
        const uint8_t reg1 = _NEXT_BYTE;
        const uint8_t reg2 = _NEXT_BYTE;
        const uint8_t size = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg1)
        _CHECK_REGISTER_VALID(reg2)
        // address is located in reg2 reg
        uint32_t addr = ctx->registers[reg2];
        bool isRam = addr & 0x80000000;
        addr &= 0xFFFF; // mask the RAM/ROM bit, ensure 16-bit addressing
        if (isRam) {
            _CHECK_RAM_ADDR_VALID(addr)
            memcpy(&ctx->registers[reg1], &ctx->ram.mem[addr], size);
        } else {
            _CHECK_ROM_ADDR_VALID(addr)
            memcpy(&ctx->registers[reg1], &ctx->rom.mem[addr], size);
        }
        break;
    }
    case OP_ADD:
    {
        const uint8_t reg1 = _NEXT_BYTE;
        const uint8_t reg2 = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg1)
        _CHECK_REGISTER_VALID(reg2)
        ctx->registers[reg1] = ctx->registers[reg1] + ctx->registers[reg2];
        break;
    }
    case OP_ADDI:
    {
        const uint8_t reg1 = _NEXT_BYTE;
        const uint8_t val = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg1)
        ctx->registers[reg1] = ctx->registers[reg1] + val;
        break;
    }
    case OP_SUB:
    {
        const uint8_t reg1 = _NEXT_BYTE;
        const uint8_t reg2 = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg1)
        _CHECK_REGISTER_VALID(reg2)
        ctx->registers[reg1] = ctx->registers[reg1] - ctx->registers[reg2];
        break;
    }
    case OP_SUBI:
    {
        const uint8_t reg1 = _NEXT_BYTE;
        const uint8_t val = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg1)
        ctx->registers[reg1] = ctx->registers[reg1] - val;
        break;
    }
    case OP_MUL:
    {
        const uint8_t reg1 = _NEXT_BYTE;
        const uint8_t reg2 = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg1)
        _CHECK_REGISTER_VALID(reg2)
        ctx->registers[reg1] = ctx->registers[reg1] * ctx->registers[reg2];
        break;
    }
    case OP_DIV:
    {
        const uint8_t reg1 = _NEXT_BYTE;
        const uint8_t reg2 = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg1)
        _CHECK_REGISTER_VALID(reg2)
        ctx->registers[reg1] = ctx->registers[reg1] / ctx->registers[reg2];
        break;
    }
    case OP_SHL:
    {
        const uint8_t reg1 = _NEXT_BYTE;
        const uint8_t reg2 = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg1)
        _CHECK_REGISTER_VALID(reg2)
        ctx->registers[reg1] = ctx->registers[reg1] << ctx->registers[reg2];
        break;
    }
    case OP_SHR:
    {
        const uint8_t reg1 = _NEXT_BYTE;
        const uint8_t reg2 = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg1)
        _CHECK_REGISTER_VALID(reg2)
        ctx->registers[reg1] = ctx->registers[reg1] >> ctx->registers[reg2];
        break;
    }
    case OP_ISHR:
    {
        const uint8_t reg1 = _NEXT_BYTE;
        const uint8_t reg2 = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg1)
        _CHECK_REGISTER_VALID(reg2)
        *((int32_t *)&ctx->registers[reg1]) = *((int32_t *)&ctx->registers[reg1]) >> *((int32_t *)&ctx->registers[reg2]);
        break;
    }
    case OP_AND:
    {
        const uint8_t reg1 = _NEXT_BYTE;
        const uint8_t reg2 = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg1)
        _CHECK_REGISTER_VALID(reg2)
        ctx->registers[reg1] = ctx->registers[reg1] & ctx->registers[reg2];
        break;
    }
    case OP_OR:
    {
        const uint8_t reg1 = _NEXT_BYTE;
        const uint8_t reg2 = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg1)
        _CHECK_REGISTER_VALID(reg2)
        ctx->registers[reg1] = ctx->registers[reg1] | ctx->registers[reg2];
        break;
    }
    case OP_XOR:
    {
        const uint8_t reg1 = _NEXT_BYTE;
        const uint8_t reg2 = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg1)
        _CHECK_REGISTER_VALID(reg2)
        ctx->registers[reg1] = ctx->registers[reg1] ^ ctx->registers[reg2];
        break;
    }
    case OP_NOT:
    {
        const uint8_t reg1 = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg1)
        ctx->registers[reg1] = ~ctx->registers[reg1];
        break;
    }
    case OP_JUMP:
    {
        ctx->registers[PC] = _NEXT_SHORT(ctx) - 1;
        break;
    }
    case OP_JUMPR:
    {
        const uint8_t reg = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg)
        ctx->registers[PC] = ctx->registers[reg] - 1;
        break;
    }
    case OP_SKIPZ:
    case OP_SKIPNZ:
    {
        const uint8_t reg = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg)
        bool skip = instr == OP_SKIPZ ? ctx->registers[reg] == 0 : ctx->registers[reg] != 0;
        if (skip)
        {
            ctx->registers[PC]++; // 1. go to next instruction
            instr = ctx->rom.mem[ctx->registers[PC]];
            bytes = OpCodes[instr].bytes;
            ctx->registers[PC] += bytes; // jump over argument bytes
        }
        break;
    }

    case OP_CMP_EQ:
    {
        const uint8_t reg1 = _NEXT_BYTE;
        const uint8_t reg2 = _NEXT_BYTE;
        const uint8_t reg3 = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg1)
        _CHECK_REGISTER_VALID(reg2)
        _CHECK_REGISTER_VALID(reg3)
        ctx->registers[reg1] = ctx->registers[reg2] == ctx->registers[reg3] ? 1 : 0;
        break;
    }
    case OP_CMP_GT:
    {
        const uint8_t reg1 = _NEXT_BYTE;
        const uint8_t reg2 = _NEXT_BYTE;
        const uint8_t reg3 = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg1)
        _CHECK_REGISTER_VALID(reg2)
        _CHECK_REGISTER_VALID(reg3)
        ctx->registers[reg1] = ctx->registers[reg2] > ctx->registers[reg3] ? 1 : 0;
        break;
    }
    case OP_CMP_LT:
    {
        const uint8_t reg1 = _NEXT_BYTE;
        const uint8_t reg2 = _NEXT_BYTE;
        const uint8_t reg3 = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg1)
        _CHECK_REGISTER_VALID(reg2)
        _CHECK_REGISTER_VALID(reg3)
        ctx->registers[reg1] = ctx->registers[reg2] < ctx->registers[reg3] ? 1 : 0;
        break;
    }
    }

    ctx->registers[PC]++;
    ctx->instrCount++;

    return result;
}
