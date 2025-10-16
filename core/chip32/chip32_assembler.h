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

#ifndef CHIP32_ASSEMBLER_H
#define CHIP32_ASSEMBLER_H

#include "chip32_vm.h"
#include <vector>
#include <cstdint>
#include <string>
#include <map>
#include <iostream>
#include <memory>

namespace Chip32
{

struct Mnemonics {
    std::string mnemonic;
    OpCode op;
};

// Complete tokenized instruction
struct Instr {
    uint16_t line{0};
    std::vector<std::string> args;
    std::vector<uint8_t> compiledArgs;
    OpCode code { 0, 0, 0 };
    std::string mnemonic;
    uint16_t dataTypeSize{0};
    uint16_t dataLen{0};

    bool isLabel{false};    //!< If true, this is a label, otherwise it is an instruction
    bool useLabel{false};   //!< If true, the instruction uses a label
    bool isRomData{false};  //!< True if constant data in ROM (DC)
    bool isRamData{false};  //!< True if RAM variable (DV or DZ)
    bool isZeroData{false}; //!< True if zero-initialized RAM (DZ only)

    uint32_t addr{0};       //!< Instruction/data address (ROM for DC, RAM for DV/DZ)

    bool isRomCode() const { return !(isLabel || isRomData || isRamData); }
};

struct RegNames
{
    chip32_register_t reg;
    std::string name;
};

class Assembler
{
public:

    struct Error {
        std::string message;
        int line{-1};
        std::string ToString() const {
            if (line < 0)
                return "No error";
            else
                return "Error line " + std::to_string(line) + ", " + message; 
        }
    };

    // Separated parser to allow only code check
    bool Parse(const std::string &data);

    void Clear() {
        m_labels.clear();
        m_instructions.clear();
    }

    static std::vector<std::string> Split(const std::string &line);

    std::vector<std::shared_ptr<Instr>>::const_iterator begin() { return m_instructions.begin(); }
    std::vector<std::shared_ptr<Instr>>::const_iterator end() { return m_instructions.end(); }

    // Returns the register number from the name
    bool GetRegister(const std::string &regName, uint8_t &reg);
    bool GetRegisterName(uint8_t reg, std::string &regName);

    Error GetLastError() { return m_lastError; }

    bool GetMain(std::shared_ptr<Instr> &instr) const {

        // Find the main label
        bool success = m_labels.count(".main") == 1;

        if (success)
        {
            instr = m_labels.at(".main");
        }

        return success;
    }

private:
    bool CompileMnemonicArguments(std::shared_ptr<Instr> instr);

    // label, shared pointer to instruction
    std::map<std::string, std::shared_ptr<Instr>> m_labels;

    Error m_lastError;

    std::vector<std::shared_ptr<Instr>> m_instructions;
    bool CompileConstantArgument(std::shared_ptr<Instr> instr, const std::string &a);
};

}

#endif // CHIP32_ASSEMBLER_H