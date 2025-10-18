// ===================================================================
// test_vm.cpp - Tests exhaustifs de toutes les instructions Chip32
// ===================================================================

#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include "chip32_machine.h"

class VmTestContext
{
public:
    VmTestContext() {}

    void Execute(const std::string &assemblyCode)
    {
        machine.QuickExecute(assemblyCode);
        REQUIRE(machine.parseResult == true);
        REQUIRE(machine.buildResult == true);
        REQUIRE(machine.runResult == VM_FINISHED);
    }

    Chip32::Machine machine;
};

// ===================================================================
// ARITHMETIC OPERATIONS
// ===================================================================

TEST_CASE_METHOD(VmTestContext, "ADD - Addition", "[vm][arithmetic][add]") {
    static const std::string test = R"(
        lcons r0, 10
        lcons r1, 32
        add r0, r1
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 42);
}

TEST_CASE_METHOD(VmTestContext, "ADDI - Addition immediate (if supported)", "[vm][arithmetic][addi]") {
    // Note: Vérifier si ADDI est implémenté dans votre VM
    static const std::string test = R"(
        lcons r0, 10
        addi r0, 32
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 42);
}

TEST_CASE_METHOD(VmTestContext, "SUB - Subtraction", "[vm][arithmetic][sub]") {
    static const std::string test = R"(
        lcons r0, 50
        lcons r1, 8
        sub r0, r1
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 42);
}

TEST_CASE_METHOD(VmTestContext, "SUBI - Subtraction immediate (if supported)", "[vm][arithmetic][subi]") {
    static const std::string test = R"(
        lcons r0, 50
        subi r0, 8
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 42);
}

TEST_CASE_METHOD(VmTestContext, "MUL - Multiplication", "[vm][arithmetic][mul]") {
    static const std::string test = R"(
        lcons r0, 37
        lcons r1, 0x695
        mul r0, r1
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 37 * 0x695);
}

TEST_CASE_METHOD(VmTestContext, "DIV - Division", "[vm][arithmetic][div]") {
    static const std::string test = R"(
        lcons r0, 84
        lcons r1, 2
        div r0, r1
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 42);
}

TEST_CASE_METHOD(VmTestContext, "DIV - Division by zero protection", "[vm][arithmetic][div]") {
    static const std::string test = R"(
        lcons r0, 42
        lcons r1, 0
        div r0, r1
        halt
    )";
    Execute(test);
    // Vérifier que la VM ne crash pas (comportement peut varier)
}

// ===================================================================
// BITWISE OPERATIONS
// ===================================================================

TEST_CASE_METHOD(VmTestContext, "AND - Bitwise AND", "[vm][bitwise][and]") {
    static const std::string test = R"(
        lcons r0, 0xFF
        lcons r1, 0x0F
        and r0, r1
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 0x0F);
}

TEST_CASE_METHOD(VmTestContext, "OR - Bitwise OR", "[vm][bitwise][or]") {
    static const std::string test = R"(
        lcons r0, 0xF0
        lcons r1, 0x0F
        or r0, r1
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 0xFF);
}

TEST_CASE_METHOD(VmTestContext, "XOR - Bitwise XOR", "[vm][bitwise][xor]") {
    static const std::string test = R"(
        lcons r0, 0xFF
        lcons r1, 0xAA
        xor r0, r1
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 0x55);
}

TEST_CASE_METHOD(VmTestContext, "NOT - Bitwise NOT", "[vm][bitwise][not]") {
    static const std::string test = R"(
        lcons r0, 0x00000000
        not r0
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 0xFFFFFFFF);
}

TEST_CASE_METHOD(VmTestContext, "SHIFTL - Shift left", "[vm][bitwise][shiftl]") {
    static const std::string test = R"(
        lcons r0, 0x01
        lcons r1, 4
        shiftl r0, r1
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 0x10);
}

TEST_CASE_METHOD(VmTestContext, "SHIFTR - Shift right (logical)", "[vm][bitwise][shiftr]") {
    static const std::string test = R"(
        lcons r0, 0x80
        lcons r1, 4
        shiftr r0, r1
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 0x08);
}

TEST_CASE_METHOD(VmTestContext, "ISHIFTR - Shift right (arithmetic)", "[vm][bitwise][ishiftr]") {
    static const std::string test = R"(
        lcons r0, 0xFFFFFF80
        lcons r1, 2
        ishiftr r0, r1
        halt
    )";
    Execute(test);
    // Shift arithmétique conserve le signe
    REQUIRE((int32_t)machine.ctx.registers[R0] == -32);
}

// ===================================================================
// DATA MOVEMENT
// ===================================================================

TEST_CASE_METHOD(VmTestContext, "MOV - Move register", "[vm][data][mov]") {
    static const std::string test = R"(
        lcons r0, 42
        mov r1, r0
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R1] == 42);
}

TEST_CASE_METHOD(VmTestContext, "LCONS - Load constant", "[vm][data][lcons]") {
    static const std::string test = R"(
        lcons r0, 0x12345678
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 0x12345678);
}

// ===================================================================
// MEMORY OPERATIONS
// ===================================================================

TEST_CASE_METHOD(VmTestContext, "STORE and LOAD - Memory operations", "[vm][memory]") {
    static const std::string test = R"(
$myVar DV32, 0

.main:
        lcons r0, 42
        lcons r1, $myVar
        store @r1, r0, 4
        
        lcons r2, 0
        load r2, @r1, 4
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R2] == 42);
}

TEST_CASE_METHOD(VmTestContext, "LOAD - Direct address", "[vm][memory][load]") {
    static const std::string test = R"(
$value DV32, 123

.main:
        load r0, $value, 4
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 123);
}

// ===================================================================
// STACK OPERATIONS
// ===================================================================

TEST_CASE_METHOD(VmTestContext, "PUSH and POP", "[vm][stack]") {
    static const std::string test = R"(
        lcons r0, 42
        lcons r1, 100
        
        push r0
        push r1
        
        lcons r0, 0
        lcons r1, 0
        
        pop r1
        pop r0
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 42);
    REQUIRE(machine.ctx.registers[R1] == 100);
}

TEST_CASE_METHOD(VmTestContext, "PUSH/POP - Multiple values", "[vm][stack]") {
    static const std::string test = R"(
        lcons r0, 1
        lcons r1, 2
        lcons r2, 3
        lcons r3, 4
        
        push r0
        push r1
        push r2
        push r3
        
        lcons r0, 0
        lcons r1, 0
        lcons r2, 0
        lcons r3, 0
        
        pop r3
        pop r2
        pop r1
        pop r0
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 1);
    REQUIRE(machine.ctx.registers[R1] == 2);
    REQUIRE(machine.ctx.registers[R2] == 3);
    REQUIRE(machine.ctx.registers[R3] == 4);
}

// ===================================================================
// CONTROL FLOW - JUMP
// ===================================================================

TEST_CASE_METHOD(VmTestContext, "JUMP - Unconditional jump", "[vm][control][jump]") {
    static const std::string test = R"(
        lcons r0, 0
        jump .target
        lcons r0, 99
.target:
        lcons r0, 42
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 42);
}

// ===================================================================
// CONTROL FLOW - SKIP
// ===================================================================

TEST_CASE_METHOD(VmTestContext, "SKIPZ - Skip if zero (condition true)", "[vm][control][skipz]") {
    static const std::string test = R"(
        lcons r0, 0
        skipz r0
        jump .non_zero
        lcons r1, 42
        halt
    .non_zero:
        lcons r1, 99
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R1] == 42);
}

TEST_CASE_METHOD(VmTestContext, "SKIPZ - Skip if zero (condition false)", "[vm][control][skipz]") {
    static const std::string test = R"(
        lcons r0, 1
        skipz r0
        jump .non_zero
        lcons r1, 42
        halt
    .non_zero:
        lcons r1, 99
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R1] == 99);
}

TEST_CASE_METHOD(VmTestContext, "SKIPNZ - Skip if not zero (condition true)", "[vm][control][skipnz]") {
    static const std::string test = R"(
        lcons r0, 1
        skipnz r0
        jump .it_is_zero
        lcons r1, 42
        halt
    .it_is_zero:
        lcons r1, 99
        halt
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R1] == 42);
}

TEST_CASE_METHOD(VmTestContext, "SKIPNZ - Skip if not zero (condition false)", "[vm][control][skipnz]") {
    static const std::string test = R"(
        lcons r0, 0
        skipnz r0
        jump .it_is_zero
        lcons r1, 42
        halt
    .it_is_zero:
        lcons r1, 99
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R1] == 99);
}

TEST_CASE_METHOD(VmTestContext, "SKIPZ - Multiple instructions", "[vm][control][skipz]") {
    static const std::string test = R"(
        lcons r0, 0
        lcons r1, 0
        skipz r0
        lcons r1, 10
        add r1, r0
        halt
    )";
    Execute(test);
    // Skip lcons, donc r1 reste 0, puis add 0 + 0 = 0
    REQUIRE(machine.ctx.registers[R1] == 0);
}

// ===================================================================
// COMPARISONS
// ===================================================================

TEST_CASE_METHOD(VmTestContext, "EQ - Equal (true)", "[vm][comparison][eq]") {
    static const std::string test = R"(
        lcons r1, 42
        lcons r2, 42
        eq r0, r1, r2
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 1);
}

TEST_CASE_METHOD(VmTestContext, "EQ - Equal (false)", "[vm][comparison][eq]") {
    static const std::string test = R"(
        lcons r1, 42
        lcons r2, 10
        eq r0, r1, r2
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 0);
}

TEST_CASE_METHOD(VmTestContext, "GT - Greater than (true)", "[vm][comparison][gt]") {
    static const std::string test = R"(
        lcons r1, 50
        lcons r2, 10
        gt r0, r1, r2
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 1);
}

TEST_CASE_METHOD(VmTestContext, "GT - Greater than (false)", "[vm][comparison][gt]") {
    static const std::string test = R"(
        lcons r1, 10
        lcons r2, 50
        gt r0, r1, r2
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 0);
}

TEST_CASE_METHOD(VmTestContext, "GT - Greater than (equal)", "[vm][comparison][gt]") {
    static const std::string test = R"(
        lcons r1, 42
        lcons r2, 42
        gt r0, r1, r2
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 0);
}

TEST_CASE_METHOD(VmTestContext, "LT - Less than (true)", "[vm][comparison][lt]") {
    static const std::string test = R"(
        lcons r1, 10
        lcons r2, 50
        lt r0, r1, r2
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 1);
}

TEST_CASE_METHOD(VmTestContext, "LT - Less than (false)", "[vm][comparison][lt]") {
    static const std::string test = R"(
        lcons r1, 50
        lcons r2, 10
        lt r0, r1, r2
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 0);
}

TEST_CASE_METHOD(VmTestContext, "LT - Less than (equal)", "[vm][comparison][lt]") {
    static const std::string test = R"(
        lcons r1, 42
        lcons r2, 42
        lt r0, r1, r2
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 0);
}

// ===================================================================
// FUNCTION CALLS
// ===================================================================

TEST_CASE_METHOD(VmTestContext, "CALL and RET - Function call", "[vm][function]") {
    static const std::string test = R"(
        lcons r0, 10
        call .myFunction
        lcons r1, 100
        halt

.myFunction:
        lcons r0, 42
        ret
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 42);
    REQUIRE(machine.ctx.registers[R1] == 100);
}

TEST_CASE_METHOD(VmTestContext, "CALL - Nested function calls", "[vm][function]") {
    static const std::string test = R"(
        lcons r0, 0
        call .func1
        halt

.func1:
        lcons r0, 1
        call .func2
        ret

.func2:
        lcons r0, 42
        ret
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 42);
}

// ===================================================================
// SPECIAL INSTRUCTIONS
// ===================================================================

TEST_CASE_METHOD(VmTestContext, "NOP - No operation", "[vm][special][nop]") {
    static const std::string test = R"(
        lcons r0, 42
        nop
        nop
        nop
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 42);
}

TEST_CASE_METHOD(VmTestContext, "HALT - Program termination", "[vm][special][halt]") {
    static const std::string test = R"(
        lcons r0, 42
        halt
        lcons r0, 99
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 42);
}

// ===================================================================
// COMPLEX SCENARIOS
// ===================================================================

TEST_CASE_METHOD(VmTestContext, "Complex - Factorial calculation", "[vm][complex]") {
    static const std::string test = R"(
; Calculate 5! = 120
        lcons r0, 5       ; n
        lcons r1, 1       ; result
        
.loop:
        mul r1, r0
        lcons r2, 1
        sub r0, r2
        skipz r0
        jump .loop
        
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R1] == 120);
}

TEST_CASE_METHOD(VmTestContext, "Complex - Fibonacci calculation", "[vm][complex]") {
    static const std::string test = R"(
; Calculate 10th Fibonacci number = 55
        lcons r0, 0       ; fib(n-2)
        lcons r1, 1       ; fib(n-1)
        lcons r2, 10      ; counter
        
.loop:
        lcons r3, 1
        sub r2, r3
        skipz r2
        jump .continue
        jump .done
        
.continue:
        mov r3, r1
        add r1, r0
        mov r0, r3
        jump .loop
        
.done:
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R1] == 55);
}

TEST_CASE_METHOD(VmTestContext, "Complex - Array sum", "[vm][complex][array]") {
    static const std::string test = R"(
$array DV32, 10, 20, 30, 40, 50

.main:
        lcons r0, 0       ; sum
        lcons r1, $array  ; pointer
        lcons r2, 5       ; count
        
.loop:
        load r3, @r1, 4
        add r0, r3
        
        lcons r4, 4
        add r1, r4
        
        lcons r4, 1
        sub r2, r4
        skipz r2
        jump .loop
        
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 150);
}

TEST_CASE_METHOD(VmTestContext, "Complex - Conditional branches", "[vm][complex][branch]") {
    static const std::string test = R"(
        lcons r0, 10
        lcons r1, 5
        
        gt r2, r0, r1
        skipz r2
        jump .greater
        
        lcons r3, 1
        jump .end
        
.greater:
        lcons r3, 2
        
.end:
        halt
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R3] == 2);
}

// ===================================================================
// EDGE CASES
// ===================================================================

TEST_CASE_METHOD(VmTestContext, "Edge - Maximum positive integer", "[vm][edge]") {
    static const std::string test = R"(
        lcons r0, 0x7FFFFFFF
        lcons r1, 1
        add r0, r1
        halt
    )";
    Execute(test);
    // Overflow vers valeur négative
    REQUIRE((int32_t)machine.ctx.registers[R0] < 0);
}

TEST_CASE_METHOD(VmTestContext, "Edge - All registers", "[vm][edge][registers]") {
    static const std::string test = R"(
        lcons r0, 0
        lcons r1, 1
        lcons r2, 2
        lcons r3, 3
        lcons r4, 4
        lcons r5, 5
        lcons r6, 6
        lcons r7, 7
        lcons r8, 8
        lcons r9, 9
        halt
    )";
    Execute(test);
    for (int i = 0; i < 10; i++) {
        REQUIRE(machine.ctx.registers[i] == i);
    }
}

TEST_CASE_METHOD(VmTestContext, "Edge - Deep nested calls", "[vm][edge][stack]") {
    static const std::string test = R"(
        call .level1
        halt

.level1:
        call .level2
        ret

.level2:
        call .level3
        ret

.level3:
        lcons r0, 42
        ret
    )";
    Execute(test);
    REQUIRE(machine.ctx.registers[R0] == 42);
}
