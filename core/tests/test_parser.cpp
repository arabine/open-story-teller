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
#include <thread>

#include "catch.hpp"
#include "chip32_assembler.h"
#include "chip32_macros.h"

/*
Purpose: grammar, ram usage and macros, rom code generation
*/

void hexdump(void *ptr, int buflen);

static const std::string test1 = R"(
; label definition
.main:   ;; comment here should work
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

$imageBird          DC8  "example.bmp", 8  ; data
$someConstant       DC32  12456789

; DSxx to declare a variable in RAM, followed by the number of elements
$RamData1           DV32    1 ; one 32-bit integer
$MyArray            DV8    10 ; array of 10 bytes

)";

#include <stdarg.h>
#include <string.h>


int get_from_memory(chip32_ctx_t *ctx, uint32_t addr, char *text)
{
    int valid = 0;

    // Test if address is valid

    bool isRam = addr & 0x80000000;
    addr &= 0xFFFF; // mask the RAM/ROM bit, ensure 16-bit addressing
    if (isRam) {
        strcpy(&text[0], (const char *)&ctx->ram.mem[addr]);
    } else {
        strcpy(&text[0], (const char *)&ctx->rom.mem[addr]);
    }

    return valid;
}


static uint8_t story_player_syscall(chip32_ctx_t *ctx, uint8_t code)
{
    uint8_t retCode = SYSCALL_RET_OK;
    char working_buf[100] = {0};

    // Printf
    if (code == 4)
    {
        // In R0: string with escaped characters
        // R1: Number of arguments
        // R2, R3 ... arguments

        // Integers: stored in registers by values
        // Strings: first character address in register

        get_from_memory(ctx, ctx->registers[R0], working_buf);
        int arg_count = ctx->registers[R1];

        switch(arg_count){
            case 0: 
                puts(working_buf);
                break;
            case 1: 
                printf(working_buf, ctx->registers[R2]);
                puts("");
                break;
            case 2: 
                printf(working_buf, ctx->registers[R2], ctx->registers[R3]);
                puts("");
                break;
            case 3: 
                printf(working_buf, ctx->registers[R2], ctx->registers[R3], ctx->registers[R4]);
                puts("");
                break;
            default:
                break;
        }
       
    }
    // WAIT (sleep)
    else if (code == 5)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ctx->registers[R0]));
    }

    return retCode;
}





TEST_CASE( "Check various indentations and typos" ) {

    std::vector<uint8_t> program;
    Chip32::Assembler assembler;
    Chip32::Result result;
    uint8_t data[8*1024];

    bool parseResult = assembler.Parse(test1);

    std::cout << assembler.GetLastError().ToString() << std::endl;

    REQUIRE( parseResult == true );

    REQUIRE( assembler.BuildBinary(program, result) == true);
    result.Print();
    hexdump(program.data(), program.size());

    chip32_ctx_t chip32_ctx;

    chip32_ctx.stack_size = 512;

    chip32_ctx.rom.mem = program.data();
    chip32_ctx.rom.addr = 0;
    chip32_ctx.rom.size = program.size();

    chip32_ctx.ram.mem = data;
    chip32_ctx.ram.addr = 40 *1024;
    chip32_ctx.ram.size = sizeof(data);

    chip32_ctx.syscall = story_player_syscall;

    chip32_initialize(&chip32_ctx);
    chip32_result_t runResult = chip32_run(&chip32_ctx);
    REQUIRE( runResult == VM_FINISHED );
}

// ====================================================================================================
static const std::string testPrintf = R"(

; ========================================================
; We test the printf system call
; ========================================================
$printHello         DC8  "La réponse est %d"
$answer             DC32  42

$counter             DV32  10

.main:
    
    ; prepapre loop

    lcons t0, 1000
    lcons t1, 4
    .while R1 > 0
        .print "La valeur est: %d", $counter

        mov r0, t0      ; wait time in ms for argument
        syscall 5       ; wait call

    .endwhile

    
    halt


    
)";


static const std::string testMacro1 = R"(

%section_macro

%macro incr 1
    push t0
    lcons t0, 1
    add %1, t0
    pop t0
%endmacro


%macro print 2
    lcons r0, %1 ; string text
    lcons r1, 1 ; number of arguments
    mov r2, %2
    syscall 4
%endmacro

%macro LOOP_START 3
    lcons %2, %3 ; Initialise le compteur de boucle (registre spécifié)
    %1_loop:      ; Étiquette de début de boucle
%endmacro


%macro LOOP_END 2
    subi %2, 1 ; Décrémente le registre spécifié
    skipz %2
    jump %1_loop ; Saute si le registre n'est pas zéro
%endmacro

%section_text

    lcons R3, 4
    incr R3

    LOOP_START .myLoop, r6, 5
    print $printHello, r3
    LOOP_END .myLoop, r6 
    halt

%section_data

$printHello         DC8  "Answer is %d"
    
)";

TEST_CASE( "Check assembly macro language part 1" )
{

    Chip32::ScriptProcessor processor;
    processor.process(testMacro1);

    processor.generate_assembly();

    std::string resultAsm = processor.GetResult();

    std::cout << "-----------------------------------------------------" << std::endl;
    std::cout << resultAsm << std::endl;
    std::cout << "-----------------------------------------------------"  << std::endl;

/*
    const std::string& output_filename
    std::ofstream out(output_filename);
    if (!out) {
        std::cerr << "Error creating file: " << output_filename << "\n";
        return;
    }

    out.close();
*/


    std::vector<uint8_t> program;
    Chip32::Assembler assembler;
    Chip32::Result result;
    uint8_t data[8*1024];

    bool parseResult = assembler.Parse(resultAsm);

    std::cout << assembler.GetLastError().ToString() << std::endl;

    REQUIRE( parseResult == true );


    REQUIRE( assembler.BuildBinary(program, result) == true);
    result.Print();
    hexdump(program.data(), program.size());

    chip32_ctx_t chip32_ctx;

    chip32_ctx.stack_size = 512;

    chip32_ctx.rom.mem = program.data();
    chip32_ctx.rom.addr = 0;
    chip32_ctx.rom.size = program.size();

    chip32_ctx.ram.mem = data;
    chip32_ctx.ram.addr = 40 *1024;
    chip32_ctx.ram.size = sizeof(data);

    chip32_ctx.syscall = story_player_syscall;

    chip32_initialize(&chip32_ctx);
    chip32_result_t runResult = chip32_run(&chip32_ctx);
    
    REQUIRE( runResult == VM_FINISHED );

    REQUIRE( chip32_ctx.registers[R3] == 5);
    

}
