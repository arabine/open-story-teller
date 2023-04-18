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
#include "chip32.h"

/*
Purpose: test all opcodes
*/

void hexdump(void *ptr, int buflen);

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

        // ---------  EXECUTE BINARY  ---------
        virtual_mem_t rom = {
            .mem = program.data(),
            .size = 8*1024,
            .addr = 18 * 1024
        };
        virtual_mem_t ram = {
            .mem = data,
            .size = sizeof(data),
            .addr = 56*1024
        };

        chip32_initialize(&rom, &ram, 256);
        chip32_result_t runResult = chip32_run(program.size(), 1000);
        REQUIRE( runResult == VM_FINISHED );
    }

    uint8_t data[8*1024];
    std::vector<uint8_t> program;
    Chip32Assembler assembler;
    AssemblyResult result;
};


TEST_CASE_METHOD(VmTestContext, "MUL", "[vm]") {
    static const std::string test1 = R"(
        lcons r0, 37
        lcons r1, 0x695
        mul r0, r1
        halt
    )";
    Execute(test1);

    uint32_t result = chip32_get_register(R0);
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

    uint32_t result = chip32_get_register(R0);
    REQUIRE (result == (int)(37/8));
}
