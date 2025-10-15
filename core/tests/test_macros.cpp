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