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

/*
Purpose: grammar, ram usage and macros, rom code generation
*/

void hexdump(void *ptr, int buflen);

static const std::string test1 = R"(; jump over the data, to our entry label
    jump         .entry

$imageBird          DC8  "example.bmp", 8  ; data
$someConstant       DC32  12456789

; DSxx to declare a variable in RAM, followed by the number of elements
$RamData1           DV32    1 ; one 32-bit integer
$MyArray            DV8    10 ; array of 10 bytes

; label definition
.entry:   ;; comment here should work
; We create a stupid loop just for RAM variable testing

    lcons r0, 4 ; prepare loop: 4 iterations
    lcons r2, $RamData1 ; save in R2 a ram address
    store @r2, r0, 4 ; save R0 in RAM
    lcons r1, 1
.loop:
    load r0, @r2, 4  ; load this variable
    sub r0, r1
    store @r2, r0, 4 ; save R0 in RAM
    skipz r0   ; skip loop if R0 == 0
    jump .loop


    mov      r0, r2  ; copy R2 into R0 (blank space between , and R2)
mov R0,R2  ; copy R2 into R0 (NO blank space between , and R2)

    halt
)";


static uint8_t story_player_syscall(chip32_ctx_t *ctx, uint8_t code)
{
    uint8_t retCode = SYSCALL_RET_OK;

    return retCode;
}

TEST_CASE( "Check various indentations and typos" ) {

    std::vector<uint8_t> program;
    Chip32::Assembler assembler;
    Chip32::Result result;
    uint8_t data[8*1024];

    bool parseResult = assembler.Parse(test1);

    std::cout << assembler.GetLastError().ToString();

    REQUIRE( parseResult == true );

    REQUIRE( assembler.BuildBinary(program, result) == true);
    result.Print();
    hexdump(program.data(), program.size());

    static chip32_ctx_t chip32_ctx;

    chip32_ctx.stack_size = 512;

    chip32_ctx.rom.mem = program.data();
    chip32_ctx.rom.addr = 0;
    chip32_ctx.rom.size = program.size();

    chip32_ctx.ram.mem = data;
    chip32_ctx.ram.addr = 40 *1024,
    chip32_ctx.ram.size = sizeof(data);

    chip32_ctx.syscall = story_player_syscall;

    chip32_initialize(&chip32_ctx);
    chip32_result_t runResult = chip32_run(&chip32_ctx);
    REQUIRE( runResult == VM_FINISHED );
}
