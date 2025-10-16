#pragma once

#include <set>
#include <stdint.h>

#include "chip32_assembler.h"
#include "chip32_vm.h"

struct DebugContext
{
    uint32_t event_mask{0};
    bool wait_event{0};
    bool free_run{false};
    uint32_t line{0};
    chip32_result_t run_result{VM_FINISHED};

    std::set<int> m_breakpoints;

    void Stop() {
        run_result = VM_FINISHED;
    }

    bool IsValidEvent(uint32_t event) {
        return (event_mask & event) != 0;
    }

    static void DumpCodeAssembler(Chip32::Assembler & assembler) 
    {

        for (auto iter : assembler)
        {
            if (iter->isRomCode() || iter->isRomData)
            {
                std::cout << "-------------------" << std::endl;
                std::cout << "Instr: " << iter->mnemonic.c_str() << std::endl;
                std::cout << "Addr: " <<  std::hex << iter->addr << std::endl;
                std::cout << "Line: " << iter->line << std::endl;
                std::cout << "\t- Opcode: "  << std::hex <<  iter->code.opcode
                         << ", opcode args: " << iter->code.bytes << std::endl;

                int i = 1;
                for (auto arg : iter->compiledArgs)
                {
                    std::cout  << "\t- Arg " << i << " : " << std::hex << arg << std::endl;
                    i++;
                }
            }
        }

    }
};
