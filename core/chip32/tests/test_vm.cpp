/*
The MIT License

Copyright (c) 2022 Anthony Rabine

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

#include <iostream>
#include "catch.hpp"
#include "chip32_assembler.h"
#include "chip32_vm.h"

/*
Purpose: test all opcodes
*/

void hexdump(void *ptr, int buflen);

static uint8_t story_player_syscall(chip32_ctx_t *ctx, uint8_t code)
{
    uint8_t retCode = SYSCALL_RET_OK;

    return retCode;
}


class VmTestContext
{
public:
    VmTestContext() {

    }

    void Execute(const std::string &assemblyCode)
    {
        // ---------  BUILD BINARY  ---------
        REQUIRE( assembler.Parse(assemblyCode) == true );
        REQUIRE( assembler.BuildBinary(program, result) == true );
        result.Print();

        chip32_ctx.stack_size = 512;

        chip32_ctx.rom.mem = program.data();
        chip32_ctx.rom.addr = 18*1024;
        chip32_ctx.rom.size = program.size();

        chip32_ctx.ram.mem = data;
        chip32_ctx.ram.addr = 56 *1024,
        chip32_ctx.ram.size = sizeof(data);

        chip32_ctx.syscall = story_player_syscall;

        chip32_initialize(&chip32_ctx);
        chip32_result_t runResult = chip32_run(&chip32_ctx);
        REQUIRE( runResult == VM_FINISHED );
    }

    uint8_t data[8*1024];
    std::vector<uint8_t> program;
    Chip32::Assembler assembler;
    Chip32::Result result;
    chip32_ctx_t chip32_ctx;
};


TEST_CASE_METHOD(VmTestContext, "MUL", "[vm]") {
    static const std::string test1 = R"(
        lcons r0, 37
        lcons r1, 0x695
        mul r0, r1
        halt
    )";
    Execute(test1);

    uint32_t result = chip32_ctx.registers[R0];
    REQUIRE (result == 37 * 0x695);
}

TEST_CASE_METHOD(VmTestContext, "DIV", "[vm]") {
    static const std::string test1 = R"(
        lcons r0, 37
        lcons r1, 8
        div r0, r1
        halt
    )";
    Execute(test1);

    uint32_t result = chip32_ctx.registers[R0];
    REQUIRE (result == (int)(37/8));
}
