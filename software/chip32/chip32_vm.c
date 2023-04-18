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
#define _NEXT_SHORT ({ ctx->registers[PC] += 2; ctx->rom.mem[ctx->registers[PC]-1]\
                     | ctx->rom.mem[ctx->registers[PC]] << 8; })
#define _NEXT_INT ({                                                                     \
    ctx->registers[PC] += 4;                                                             \
    ctx->rom.mem[ctx->registers[PC] - 3] | ctx->rom.mem[ctx->registers[PC] - 2] << 8 |   \
    ctx->rom.mem[ctx->registers[PC] - 1] << 16 | ctx->rom.mem[ctx->registers[PC]] << 24; \
})

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
    if (ctx->registers[SP] - (n * sizeof(uint32_t)) > ctx->ram.addr) \
        return VM_ERR_STACK_OVERFLOW;
#define _CHECK_CAN_POP(n)                                               \
    if (ctx->registers[SP] + (n * sizeof(uint32_t)) > (ctx->ram.addr + ctx->ram.size)) \
        return VM_ERR_STACK_UNDERFLOW;                      \
    if (ctx->registers[SP] < ctx->prog_size)                          \
        return VM_ERR_STACK_OVERFLOW;
#else
#define _CHECK_ROM_ADDR_VALID(a)
#define _CHECK_BYTES_AVAIL(n)
#define _CHECK_REGISTER_VALID(r)
#define _CHECK_CAN_PUSH(n)
#define _CHECK_CAN_POP(n)
#endif


static const OpCode OpCodes[] = OPCODES_LIST;
static const uint16_t OpCodesSize = sizeof(OpCodes) / sizeof(OpCodes[0]);

// =======================================================================================
// FUNCTIONS
// =======================================================================================
void chip32_initialize(chip32_ctx_t *ctx)
{
    memset(ctx->ram.mem, 0, ctx->ram.size);
    memset(ctx->registers, 0, REGISTER_COUNT * sizeof(uint32_t));

    ctx->skip_next = false;
    ctx->instrCount = 0;

    ctx->registers[SP] = ctx->ram.size;
}

#define MEM_ACCESS(addr, vmem) if ((addr >= vmem->addr) && ((addr + vmem->size) < vmem->size))\
{\
    addr -= vmem->addr;\
    return &vmem->mem[addr];\
}


/*

uint8_t *chip32_memory(uint16_t addr)
{
    static uint8_t dummy = 0;
    // Beware, can provoke memory overflow
    MEM_ACCESS(addr, g_rom);
    MEM_ACCESS(addr, g_ram);

    return g_ram->mem; //!< Defaut memory to RAM location if address out of segment.
}

uint32_t chip32_stack_count()
{
    return g_ram->size - ctx->registers[SP];
}

void chip32_stack_push(uint32_t value)
{
    ctx->registers[SP] -= 4;
    memcpy(chip32_memory(ctx->registers[SP]), &value, sizeof(uint32_t));
}

uint32_t chip32_stack_pop()
{
    uint32_t val = 0;
    memcpy(&val, chip32_memory(ctx->registers[SP]), sizeof(uint32_t));
    ctx->registers[SP] += 4;
    return val;
}*/


chip32_result_t chip32_run(chip32_ctx_t *ctx)
{
    chip32_result_t result = VM_OK;
    while ((ctx->max_instr == 0) || (ctx->instrCount < ctx->max_instr))
    {
        chip32_step(ctx);
    }
    return result;
}

chip32_result_t chip32_step(chip32_ctx_t *ctx)
{
    chip32_result_t result = VM_OK;

    _CHECK_ROM_ADDR_VALID(ctx->registers[PC])
    const uint8_t instr = ctx->rom.mem[ctx->registers[PC]];
    if (instr >= INSTRUCTION_COUNT)
        return VM_ERR_UNKNOWN_OPCODE;

    uint8_t bytes = OpCodes[instr].bytes;
    _CHECK_BYTES_AVAIL(bytes);

    if (ctx->skip_next)
    {
        ctx->skip_next = false;
        ctx->registers[PC] += bytes + 1; // jump over arguments and point to the next instruction
        ctx->instrCount++;
        return VM_SKIPED;
    }

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
            if (ctx->syscall(code) != 0)
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
        ctx->registers[reg] = _NEXT_INT;
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
        ctx->registers[SP] -= 4;
        memcpy(&ctx->ram.mem[ctx->registers[SP]], &ctx->registers[reg], sizeof(uint32_t));
        break;
    }
    case OP_POP:
    {
        const uint8_t reg = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg)
        _CHECK_CAN_POP(1)
        memcpy(&ctx->registers[reg], &ctx->ram.mem[ctx->registers[SP]], sizeof(uint32_t));
        ctx->registers[SP] += 4;
        break;
    }
    case OP_CALL:
    {
        ctx->registers[RA] = ctx->registers[PC] + 3;
        ctx->registers[PC] = _NEXT_SHORT - 1;
        break;
    }
    case OP_RET:
    {
        ctx->registers[PC] = ctx->registers[RA] - 1;
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
    case OP_SUB:
    {
        const uint8_t reg1 = _NEXT_BYTE;
        const uint8_t reg2 = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg1)
        _CHECK_REGISTER_VALID(reg2)
        ctx->registers[reg1] = ctx->registers[reg1] - ctx->registers[reg2];
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
    case OP_JMP:
    {
        ctx->registers[PC] = _NEXT_SHORT - 1;
        break;
    }
    case OP_JR:
    {
        const uint8_t reg1 = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg1)
        uint16_t addr = ctx->registers[reg1];
        ctx->registers[PC] = addr;
        break;
    }
    case OP_SKIPZ:
    {
        const uint8_t reg = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg)
        if (reg == 0)
        {
            ctx->skip_next = true;
        }
        break;
    }
    case OP_SKIPNZ:
    {
        const uint8_t reg = _NEXT_BYTE;
        _CHECK_REGISTER_VALID(reg)
        if (reg != 0)
        {
            ctx->skip_next = true;
        }
        break;
    }
    }

    ctx->registers[PC]++;
    ctx->instrCount++;

    return result;
}
