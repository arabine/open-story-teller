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
#include "chip32_machine.h" // Inclure chip32_machine.h au lieu de assembler et vm

/*
Purpose: test all opcodes
*/

// Suppression des fonctions et classes de configuration de la VM
// qui sont maintenant encapsulées dans Chip32::Machine.

class VmTestContext
{
public:
    VmTestContext() {
        // La RAM est allouée et initialisée à l'intérieur de QuickExecute 
        // de la classe Machine. On peut laisser le constructeur vide.
    }

    void Execute(const std::string &assemblyCode)
    {
        // Utiliser la méthode QuickExecute de Machine pour Parse, Build et Run
        machine.QuickExecute(assemblyCode);

        // Vérification de base: le parsing et le build doivent réussir
        REQUIRE( machine.parseResult == true );
        REQUIRE( machine.buildResult == true );

        // Vérification que l'exécution a fini normalement (HALT)
        REQUIRE( machine.runResult == VM_FINISHED );
    }

    Chip32::Machine machine; // Instance de la Machine à utiliser pour les tests
};


TEST_CASE_METHOD(VmTestContext, "MUL", "[vm]") {
    static const std::string test1 = R"(
        lcons r0, 37
        lcons r1, 0x695
        mul r0, r1
        halt
    )";
    Execute(test1);

    // Accéder directement aux registres de la Machine pour vérifier le résultat
    uint32_t result = machine.ctx.registers[R0];
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

    // Accéder directement aux registres de la Machine pour vérifier le résultat
    uint32_t result = machine.ctx.registers[R0];
    REQUIRE (result == (int)(37/8));
}
