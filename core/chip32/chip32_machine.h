#pragma once



#include <iostream>
#include <thread>
#include <stdarg.h>
#include <string.h>

#include "chip32_assembler.h"
#include "chip32_macros.h"

namespace Chip32
{

class Machine
{
public:
    bool parseResult{false};
    bool buildResult{false};
    chip32_result_t runResult{VM_OK};

    static int get_from_memory(chip32_ctx_t *ctx, uint32_t addr, char *text)
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



    void QuickExecute(const std::string &assemblyCode)
    {
        std::vector<uint8_t> program;
        Chip32::Assembler assembler;
        Chip32::Result result;
        uint8_t data[8*1024];

        parseResult = assembler.Parse(assemblyCode);

        std::cout << assembler.GetLastError().ToString() << std::endl;


        buildResult = assembler.BuildBinary(program, result);
        result.Print();

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

        Instr mainLine;
        if (assembler.GetMain(mainLine))
        {
            // set pointer counter to the main line
            chip32_ctx.registers[PC] = mainLine.addr;
        }

        runResult = chip32_run(&chip32_ctx);
    }


};


} // namespace Chip32
