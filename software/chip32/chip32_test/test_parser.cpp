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
    store $RamData1, r0 ; save R0 in RAM
    lcons r1, 1
.loop:
    load r0, $RamData1  ; load this variable
    sub r0, r1
    store $RamData1, r0 ; save R0 in RAM
    skipz r0   ; skip loop if R0 == 0
    jump .loop


    mov      r0, r2  ; copy R2 into R0 (blank space between , and R2)
mov R0,R2  ; copy R2 into R0 (NO blank space between , and R2)

    halt
)";


TEST_CASE( "Check various indentations and typos" ) {

    std::vector<uint8_t> program;
    Chip32Assembler assembler;
    AssemblyResult result;
    uint8_t data[8*1024];

    REQUIRE( assembler.Parse(test1) == true );

    REQUIRE( assembler.BuildBinary(program, result) == true);
    result.Print();
    hexdump(program.data(), program.size());

    // ---------  EXECUTE BINARY  ---------
    virtual_mem_t rom = {
        .mem = program.data(),
        .size = 8*1024,
        .addr = 0
    };
    virtual_mem_t ram = {
        .mem = data,
        .size = sizeof(data),
        .addr = 40 *1024
    };

    chip32_initialize(&rom, &ram, 256);
    chip32_result_t runResult = chip32_run(program.size(), 1000);
    REQUIRE( runResult == VM_FINISHED );
}
