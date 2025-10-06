#pragma once



#include <iostream>
#include <thread>
#include <stdarg.h>
#include <string.h>

#include "chip32_assembler.h"
#include "chip32_macros.h"

// Dans chip32_machine.h

namespace Chip32
{

class Machine
{
public:
    bool parseResult{false};
    bool buildResult{false};
    chip32_result_t runResult{VM_OK};
    std::string printOutput;

    static Machine *m_instance;

    Machine() {
        // Bind syscall handler to this instance
        m_syscallHandler = std::bind(&Machine::HandleSyscall, this, 
                                     std::placeholders::_1, 
                                     std::placeholders::_2);
    }

    // Lecture d'une chaîne depuis la mémoire (non statique maintenant)
    static std::string GetStringFromMemory(chip32_ctx_t *ctx, uint32_t addr)
    {
        if (!ctx) {
            throw std::runtime_error("Invalid context in GetStringFromMemory");
        }

        bool isRam = (addr & 0x80000000) != 0;
        addr &= 0xFFFF;

        const uint8_t* source_mem = nullptr;
        size_t mem_size = 0;

        if (isRam) {
            if (addr >= ctx->ram.size) {
                throw std::out_of_range("RAM address out of bounds: " + std::to_string(addr));
            }
            source_mem = ctx->ram.mem;
            mem_size = ctx->ram.size;
        } else {
            if (addr >= ctx->rom.size) {
                throw std::out_of_range("ROM address out of bounds: " + std::to_string(addr));
            }
            source_mem = ctx->rom.mem;
            mem_size = ctx->rom.size;
        }

        size_t max_len = mem_size - addr;
        const char* str_start = reinterpret_cast<const char*>(&source_mem[addr]);
        
        const char* null_pos = static_cast<const char*>(
            std::memchr(str_start, '\0', max_len)
        );
        
        if (null_pos) {
            return std::string(str_start, null_pos - str_start);
        } else {
            return std::string(str_start, max_len);
        }
    }

    static std::string FormatStringWithPlaceholders(chip32_ctx_t *ctx, 
                                             const std::string& format, 
                                             const std::vector<uint32_t>& args)
    {
        std::ostringstream result;
        size_t pos = 0;
        
        while (pos < format.length()) {
            // Chercher le prochain placeholder '{'
            if (format[pos] == '{' && pos + 1 < format.length()) {
                char nextChar = format[pos + 1];
                
                // Vérifier si c'est un placeholder valide {0} à {3}
                if (nextChar >= '0' && nextChar <= '3') {
                    int argIndex = nextChar - '0';
                    
                    // Vérifier si on a assez d'arguments
                    if (argIndex >= static_cast<int>(args.size())) {
                        result << "{" << argIndex << ":?}"; // Argument manquant
                        pos += 2;
                        continue;
                    }
                    
                    uint32_t argValue = args[argIndex];
                    
                    // Vérifier s'il y a un type spécifié {:d}, {:s}, {:f}, {:x}
                    if (pos + 3 < format.length() && format[pos + 2] == ':') {
                        char typeChar = format[pos + 3];
                        
                        // Vérifier si le placeholder se termine bien par '}'
                        if (pos + 4 < format.length() && format[pos + 4] == '}') {
                            // Parser le type et formater
                            switch (typeChar) {
                                case 'd':  // Entier décimal signé
                                case 'i':
                                    result << static_cast<int32_t>(argValue);
                                    break;
                                    
                                case 'u':  // Entier non signé
                                    result << argValue;
                                    break;
                                    
                                case 'x':  // Hexadécimal minuscule
                                    result << "0x" << std::hex << argValue << std::dec;
                                    break;
                                    
                                case 'X':  // Hexadécimal majuscule
                                    result << "0x" << std::hex << std::uppercase 
                                          << argValue << std::nouppercase << std::dec;
                                    break;
                                    
                                case 's':  // String (adresse)
                                    try {
                                        result << GetStringFromMemory(ctx, argValue);
                                    } catch (const std::exception& e) {
                                        result << "<error:0x" << std::hex << argValue << std::dec << ">";
                                    }
                                    break;
                                    
                                case 'f':  // Float
                                {
                                    float floatValue;
                                    std::memcpy(&floatValue, &argValue, sizeof(float));
                                    result << floatValue;
                                    break;
                                }
                                
                                case 'c':  // Caractère
                                    result << static_cast<char>(argValue);
                                    break;
                                    
                                default:
                                    // Type inconnu, afficher tel quel
                                    result << "{" << argIndex << ":" << typeChar << "}";
                            }
                            
                            pos += 5; // Avancer de "{0:d}"
                            continue;
                        }
                    }
                    // Format court {0} sans type → défaut: entier
                    else if (pos + 2 < format.length() && format[pos + 2] == '}') {
                        result << static_cast<int32_t>(argValue);
                        pos += 3; // Avancer de "{0}"
                        continue;
                    }
                }
            }
            
            // Caractère normal, copier tel quel
            result << format[pos];
            pos++;
        }
        
        return result.str();
    }

    // Handler de syscall (méthode membre, non statique)
    uint8_t HandleSyscall(chip32_ctx_t *ctx, uint8_t code)
    {
        try {
            if (code == 4) // Printf
            {
                std::string format = GetStringFromMemory(ctx, ctx->registers[R0]);
                int arg_count = ctx->registers[R1];
                
                std::vector<uint32_t> args;
                for (int i = 0; i < arg_count && i < 4; ++i) {
                    args.push_back(ctx->registers[R2 + i]);
                }
                
                printOutput = FormatStringWithPlaceholders(ctx, format, args);
                std::cout << "[SYSCALL PRINT] " << printOutput << std::endl;
            }
            else if (code == 5) // WAIT
            {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(ctx->registers[R0])
                );
            }
            else
            {
                std::cerr << "Unknown syscall code: " << static_cast<int>(code) << std::endl;
                return SYSCALL_RET_ERROR;
            }
            
            return SYSCALL_RET_OK;
            
        } catch (const std::exception& e) {
            std::cerr << "Syscall error: " << e.what() << std::endl;
            return SYSCALL_RET_ERROR;
        }
    }

    void QuickExecute(const std::string &assemblyCode)
    {
        std::vector<uint8_t> program;
        Chip32::Assembler assembler;
        Chip32::Result result;
        std::vector<uint8_t> data(8*1024);

        parseResult = assembler.Parse(assemblyCode);
        std::cout << assembler.GetLastError().ToString() << std::endl;

        buildResult = assembler.BuildBinary(program, result);
        result.Print();

        chip32_ctx_t chip32_ctx;
        chip32_ctx.stack_size = 512;
        chip32_ctx.rom.mem = program.data();
        chip32_ctx.rom.addr = 0;
        chip32_ctx.rom.size = program.size();
        chip32_ctx.ram.mem = data.data();
        chip32_ctx.ram.addr = 40 * 1024;
        chip32_ctx.ram.size = data.size();

        // Utiliser le wrapper statique qui appelle notre fonction membre
        chip32_ctx.syscall = SyscallWrapper;
        chip32_ctx.user_data = this; // Stocker le pointeur vers cette instance

        chip32_initialize(&chip32_ctx);

        Instr mainLine;
        if (assembler.GetMain(mainLine)) {
            chip32_ctx.registers[PC] = mainLine.addr;
        }

        runResult = chip32_run(&chip32_ctx);
    }

private:
    // std::function contenant le bind
    std::function<uint8_t(chip32_ctx_t*, uint8_t)> m_syscallHandler;

    // Wrapper statique qui récupère l'instance et appelle la méthode membre
    static uint8_t SyscallWrapper(chip32_ctx_t *ctx, uint8_t code)
    {
        if (!ctx || !ctx->user_data) {
            return SYSCALL_RET_ERROR;
        }
        
        Machine* instance = static_cast<Machine*>(ctx->user_data);
        return instance->HandleSyscall(ctx, code);
    }
};

} // namespace Chip32
