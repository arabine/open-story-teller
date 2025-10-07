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
#include "chip32_machine.h"
#include "chip32_binary_format.h"

/*
Purpose: grammar, ram usage and macros, rom code generation
Tests updated with new DV/DZ syntax:
  - DV8, DV32 : RAM variables WITH initial value
  - DZ8, DZ32 : RAM zero-initialized areas (buffers)
*/

void hexdump(void *ptr, int buflen);

// ============================================================================
// TEST 1: Basic grammar with DV/DZ
// ============================================================================
static const std::string test1 = R"(
; ============================================================================
; Test grammar with new DV/DZ syntax
; ============================================================================

; ROM constants (DC)
$imageBird          DC8  "example.bmp"
$someConstant       DC32 12456789

; RAM initialized variables (DV)
$RamData1           DV32 0              ; Integer initialized to 0
$counter            DV32 10             ; Counter initialized to 10

; RAM zeroed areas (DZ)
$MyArray            DZ8  10             ; Array of 10 bytes (zeroed)
$WorkBuffer         DZ8  64             ; Buffer of 64 bytes (zeroed)

; ============================================================================
; CODE
; ============================================================================

.main:
    ; We create a loop for RAM variable testing
    lcons r0, 4                 ; prepare loop: 4 iterations
    lcons r2, $RamData1         ; save in R2 a ram address
    store @r2, r0, 4            ; save R0 in RAM
    lcons r1, 1
    
.loop:
    load r0, @r2, 4             ; load this variable
    sub r0, r1
    store @r2, r0, 4            ; save R0 in RAM
    skipz r0                    ; skip loop if R0 == 0
    jump .loop

    ; Test spacing variations
    mov      r0, r2             ; copy R2 into R0 (blank space)
    mov R0,R2                   ; copy R2 into R0 (NO blank space)

    halt
)";

// ============================================================================
// TEST CASE 1: Grammar and indentation with DV/DZ
// ============================================================================

TEST_CASE("Check various indentations and typos with DV/DZ")
{
    std::cout << "\n=== Test 1: Grammar and indentation ===" << std::endl;
    
    Chip32::Machine machine;
    machine.QuickExecute(test1);
    
    // Verify results
    REQUIRE(machine.parseResult == true);
    REQUIRE(machine.buildResult == true);
    REQUIRE(machine.runResult == VM_FINISHED);
    
    std::cout << "✓ Test 1 passed: Grammar and DV/DZ syntax" << std::endl;
}

// ============================================================================
// TEST 2: Printf with DV variable
// ============================================================================

static const std::string testPrintf = R"(
; ============================================================================
; Test printf system call with DV variable
; ============================================================================

; ROM constants (DC)
$printHello         DC8  "La réponse est %d"
$answer             DC32 42

; RAM initialized variable (DV)
$counter            DV32 10             ; Counter initialized to 10

; ============================================================================
; CODE
; ============================================================================

.main:
    ; Simple test - print the counter value
    lcons r0, $printHello
    lcons r1, 1                 ; 1 argument
    load r2, $counter, 4        ; Load counter value
    syscall 4                   ; Printf
    
    halt
)";

TEST_CASE("Test printf with DV variable")
{
    std::cout << "\n=== Test 2: Printf with DV ===" << std::endl;
    
    Chip32::Machine machine;
    machine.QuickExecute(testPrintf);
    
    REQUIRE(machine.parseResult == true);
    REQUIRE(machine.buildResult == true);
    REQUIRE(machine.runResult == VM_FINISHED);
    
    std::cout << "✓ Test 2 passed: Printf with DV" << std::endl;
}

// ============================================================================
// TEST 3: Macro language with DV/DZ
// ============================================================================

static const std::string testMacro1 = R"(
%section_macro

%macro incr 1
    push t0
    lcons t0, 1
    add %1, t0
    pop t0
%endmacro

%macro print 2
    lcons r0, %1            ; string text
    lcons r1, 1             ; number of arguments
    mov r2, %2
    syscall 4
%endmacro

%macro LOOP_START 3
    lcons %2, %3            ; Initialize loop counter
    %1_loop:                ; Loop start label
%endmacro

%macro LOOP_END 2
    subi %2, 1              ; Decrement counter
    skipz %2
    jump %1_loop            ; Jump if not zero
%endmacro

%section_text

    lcons R3, 4
    incr R3

    LOOP_START .myLoop, r6, 5
    print $printHello, r3
    LOOP_END .myLoop, r6 
    
    halt

%section_data

; ROM constant (DC)
$printHello         DC8  "Answer is %d"

; RAM zeroed buffer (DZ)
$tempBuffer         DZ8  32             ; 32 bytes buffer for temporary data
)";

TEST_CASE("Check assembly macro language with DV/DZ")
{
    std::cout << "\n=== Test 3: Macros with DV/DZ ===" << std::endl;
    
    Chip32::ScriptProcessor processor;
    processor.process(testMacro1);
    processor.generate_assembly();

    std::string resultAsm = processor.GetResult();

    std::cout << "Generated Assembly:" << std::endl;
    std::cout << resultAsm << std::endl;

    Chip32::Machine machine;
    machine.QuickExecute(resultAsm);

    REQUIRE(machine.parseResult == true);
    REQUIRE(machine.buildResult == true);
    REQUIRE(machine.runResult == VM_FINISHED);
    REQUIRE(machine.ctx.registers[R3] == 5);
    
    std::cout << "✓ Test 3 passed: Macros with DV/DZ" << std::endl;
}

// ============================================================================
// TEST 4: DV vs DZ comprehensive test
// ============================================================================

static const std::string testDvVsDz = R"(
; ============================================================================
; Comprehensive test: DV (initialized) vs DZ (zeroed)
; ============================================================================

; ROM constants (DC)
$appName            DC8  "TestApp"
$version            DC32 100

; RAM initialized variables (DV) - WITH VALUES
$counter            DV32 42             ; int counter = 42
$temperature        DV32 20             ; int temp = 20
$userName           DV8  "Guest"        ; char name[] = "Guest"
$flags              DV8  1, 0, 1        ; uint8_t flags[] = {1, 0, 1}

; RAM zeroed areas (DZ) - BUFFERS AND ARRAYS
$rxBuffer           DZ8  128            ; uint8_t buffer[128] = {0}
$dataArray          DZ32 50             ; int32_t array[50] = {0}
$workArea           DZ8  256            ; uint8_t work[256] = {0}

; ============================================================================
; CODE
; ============================================================================

.main:
    ; Test 1: DV counter should be 42
    load r0, $counter, 4
    ; r0 should be 42
    
    ; Test 2: DV temperature should be 20
    load r1, $temperature, 4
    ; r1 should be 20
    
    ; Test 3: DZ rxBuffer[0] should be 0
    lcons r2, $rxBuffer
    load r3, @r2, 1
    ; r3 should be 0
    
    ; Test 4: Write to DZ buffer
    lcons r4, 0x42
    store @r2, r4, 1
    ; rxBuffer[0] = 0x42
    
    ; Test 5: Read back modified value
    load r5, @r2, 1
    ; r5 should be 0x42
    
    halt
)";

TEST_CASE("DV vs DZ comprehensive test")
{
    std::cout << "\n=== Test 4: DV vs DZ comprehensive ===" << std::endl;
    
    Chip32::Machine machine;
    machine.QuickExecute(testDvVsDz);

    REQUIRE(machine.parseResult == true);
    REQUIRE(machine.buildResult == true);
    REQUIRE(machine.runResult == VM_FINISHED);
    
    // Verify register values
    std::cout << "\nRegister values after execution:" << std::endl;
    std::cout << "  R0 (counter) = " << machine.ctx.registers[R0] 
              << " (expected: 42)" << std::endl;
    std::cout << "  R1 (temperature) = " << machine.ctx.registers[R1] 
              << " (expected: 20)" << std::endl;
    std::cout << "  R3 (rxBuffer[0] initial) = " << machine.ctx.registers[R3] 
              << " (expected: 0)" << std::endl;
    std::cout << "  R5 (rxBuffer[0] after write) = " << machine.ctx.registers[R5] 
              << " (expected: 66)" << std::endl;
    
    REQUIRE(machine.ctx.registers[R0] == 42);   // counter DV value
    REQUIRE(machine.ctx.registers[R1] == 20);   // temperature DV value
    REQUIRE(machine.ctx.registers[R3] == 0);    // rxBuffer DZ initial (zero)
    REQUIRE(machine.ctx.registers[R5] == 0x42); // rxBuffer after write
    
    std::cout << "✓ Test 4 passed: DV vs DZ comprehensive" << std::endl;
}

// ============================================================================
// TEST 5: Binary format validation
// ============================================================================

static const std::string testBinaryFormat = R"(
; Test binary format with all section types

; DC: ROM constants
$romString      DC8  "Hello"
$romValue       DC32 123

; DV: Initialized RAM variables
$ramCounter     DV32 99
$ramMessage     DV8  "OK"

; DZ: Zeroed RAM areas
$ramBuffer      DZ8  64
$ramArray       DZ32 20

.main:
    load r0, $ramCounter, 4
    halt
)";

TEST_CASE("Binary format validation")
{
    std::cout << "\n=== Test 5: Binary format validation ===" << std::endl;
    
    Chip32::Machine machine;
    
    // Parse and build
    Chip32::Assembler assembler;
    Chip32::Result result;
    std::vector<uint8_t> program;
    
    REQUIRE(assembler.Parse(testBinaryFormat) == true);
    REQUIRE(assembler.BuildBinary(program, result) == true);
    
    std::cout << "\nBinary statistics:" << std::endl;
    result.Print();
    
    // Validate binary format
    chip32_loaded_binary_t loaded;
    chip32_binary_error_t error = chip32_binary_load(
        program.data(),
        static_cast<uint32_t>(program.size()),
        &loaded
    );
    
    REQUIRE(error == CHIP32_BIN_OK);
    
    std::cout << "\nBinary header:" << std::endl;
    chip32_binary_print_header(&loaded.header);
    
    // Verify header values
    REQUIRE(loaded.header.magic == CHIP32_MAGIC);
    REQUIRE(loaded.header.version == CHIP32_VERSION);
    REQUIRE((loaded.header.flags & CHIP32_FLAG_HAS_INIT_DATA) != 0);
    REQUIRE(loaded.header.data_size > 0);      // Has ROM constants
    REQUIRE(loaded.header.bss_size > 0);       // Has RAM
    REQUIRE(loaded.header.code_size > 0);      // Has code
    REQUIRE(loaded.header.init_data_size == loaded.header.bss_size); // Must match
    
    std::cout << "\nBinary format validations:" << std::endl;
    std::cout << "  ✓ Magic number correct" << std::endl;
    std::cout << "  ✓ Version correct" << std::endl;
    std::cout << "  ✓ Has init data flag set" << std::endl;
    std::cout << "  ✓ All sections present" << std::endl;
    std::cout << "  ✓ INIT DATA size matches BSS size" << std::endl;
    
    // Initialize RAM and verify
    std::vector<uint8_t> ram(loaded.header.bss_size);
    uint32_t init_bytes = chip32_binary_init_ram(&loaded, ram.data(), ram.size());
    
    REQUIRE(init_bytes == loaded.header.bss_size);
    
    // Verify DV ramCounter is initialized to 99
    uint32_t ramCounter = *reinterpret_cast<uint32_t*>(&ram[0]);
    std::cout << "  ✓ DV ramCounter = " << ramCounter << " (expected: 99)" << std::endl;
    REQUIRE(ramCounter == 99);
    
    std::cout << "\n✓ Test 5 passed: Binary format validation" << std::endl;
}

