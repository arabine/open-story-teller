/*
 * Chip32 Machine - VM wrapper with binary format support
 * Updated to use chip32_binary_format for proper DV/DZ handling
 */

#pragma once

#include <iostream>
#include <thread>
#include <stdarg.h>
#include <string.h>
#include <sstream>
#include <functional>
#include <cstring>

#include "chip32_assembler.h"
#include "chip32_binary_format.h"
#include "chip32_macros.h"

namespace Chip32
{

class Machine
{
public:
    bool parseResult{false};
    bool buildResult{false};
    chip32_result_t runResult{VM_OK};
    std::string printOutput;
    chip32_ctx_t ctx;  // Public pour accès aux registres dans les tests
    
    Machine() {
        // Bind syscall handler to this instance
        m_syscallHandler = std::bind(&Machine::HandleSyscall, this, 
                                     std::placeholders::_1, 
                                     std::placeholders::_2);
        m_ram.resize(1024); 
    }

    // ========================================================================
    // Méthode principale : Parse, Build, Execute
    // ========================================================================
    
    void QuickExecute(const std::string &assemblyCode)
    {
        std::vector<uint8_t> program;
        Chip32::Assembler assembler;
        Chip32::Result result;

        // Parse
        parseResult = assembler.Parse(assemblyCode);
        
        if (!parseResult) {
            std::cout << "Parse error: " << assembler.GetLastError().ToString() << std::endl;
            return;
        }

        // Build binary with new format
        buildResult = assembler.BuildBinary(program, result);
        
        if (!buildResult) {
            std::cout << "Build error: " << assembler.GetLastError().ToString() << std::endl;
            return;
        }
        
        result.Print();

        // Load binary using executable format
        chip32_binary_stats_t stats;
        chip32_binary_error_t error = chip32_binary_load(
            &ctx,
            program.data(),
            static_cast<uint32_t>(program.size()),
            m_ram.data(),
            static_cast<uint32_t>(m_ram.size()),
            &stats
        );
        
        if (error != CHIP32_BIN_OK) {
            std::cout << "Binary load error: " << chip32_binary_error_string(error) << std::endl;
            buildResult = false;
            return;
        }
        
        // Set syscall handler using wrapper
        ctx.syscall = SyscallWrapper;
        ctx.user_data = this;

        // Initialize VM
        chip32_initialize(&ctx);


        std::cout << "Starting execution at PC=0x" << std::hex << ctx.registers[PC] 
                  << std::dec << std::endl;

        // Run
        runResult = chip32_run(&ctx);
        
        std::cout << "Execution finished with result: " << runResult << std::endl;
    }

    // ========================================================================
    // Helper functions
    // ========================================================================
    
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
                        result << "{" << argIndex << ":?}";
                        pos += 3;
                        continue;
                    }
                    
                    // Vérifier le type (i ou s)
                    if (pos + 2 < format.length() && format[pos + 2] == ':') {
                        if (pos + 3 < format.length()) {
                            char typeChar = format[pos + 3];
                            uint32_t argValue = args[argIndex];
                            
                            if (typeChar == 's') {
                                // String: argValue est une adresse
                                try {
                                    std::string str = GetStringFromMemory(ctx, argValue);
                                    result << str;
                                    pos += 5; // Avancer de "{0:s}"
                                    continue;
                                } catch (const std::exception& e) {
                                    result << "{str_error}";
                                    pos += 5;
                                    continue;
                                }
                            } else if (typeChar == 'i' || typeChar == 'd') {
                                // Integer
                                result << static_cast<int32_t>(argValue);
                                pos += 5;
                                continue;
                            }
                        }
                    } else if (pos + 2 < format.length() && format[pos + 2] == '}') {
                        // Format simple {0} - traiter comme int par défaut
                        uint32_t argValue = args[argIndex];
                        result << static_cast<int32_t>(argValue);
                        pos += 3;
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

    // ========================================================================
    // Syscall handler
    // ========================================================================
    
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

    // ============================================================================
    // Fonction helper pour charger un binaire Chip32 dans la VM
    // IMPORTANT: Le vecteur binary doit rester en vie pendant toute l'exécution
    // car vm_ctx pointe directement dans ses données
    // ============================================================================
    bool LoadBinaryIntoVM(
        std::vector<uint8_t> &binary,
        chip32_ctx_t &vm_ctx,
        uint8_t *ram_buffer,
        uint32_t ram_size)
    {
       
        if (err != CHIP32_BIN_OK)
        {
            std::cerr << "Binary load error: " << chip32_binary_error_string(err) << std::endl;
            return false;
        }
        
        // Afficher les informations du binaire (debug)
        chip32_binary_print_header(&loaded.header);
        
        // Vérifier que la RAM est suffisante
        if (loaded.header.bss_size > ram_size)
        {
            std::cerr << "Insufficient RAM: need " << loaded.header.bss_size 
                    << " bytes, have " << ram_size << " bytes" << std::endl;
            return false;
        }
        
        // ========================================================================
        // Configurer la VM - ROM pointe directement dans le binaire
        // ========================================================================
        
        // Pour la VM, on considère DATA + CODE comme une seule ROM
        // DATA commence à l'offset après le header
        // CODE suit immédiatement après DATA
        
        vm_ctx.rom.mem = const_cast<uint8_t*>(loaded.data_section);
        vm_ctx.rom.addr = 0;
        vm_ctx.rom.size = loaded.header.data_size + loaded.header.code_size;
        
        // ========================================================================
        // Initialiser la RAM avec INIT DATA section
        // ========================================================================
        
        // D'abord mettre toute la RAM à zéro
        memset(ram_buffer, 0, ram_size);
        
        // Ensuite copier les données d'initialisation (DV values + DZ zeros)
        if (loaded.header.init_data_size > 0)
        {
            uint32_t copied = chip32_binary_init_ram(&loaded, ram_buffer, ram_size);
            std::cout << "Copied " << copied << " bytes of init data to RAM" << std::endl;
        }
        
        // Configurer RAM dans la VM
        vm_ctx.ram.mem = ram_buffer;
        vm_ctx.ram.addr = 0x8000; // Bit 31 = 1 pour RAM
        vm_ctx.ram.size = ram_size;
        
        // ========================================================================
        // Configurer le point d'entrée
        // ========================================================================
        // Le PC doit pointer dans la section CODE, qui commence après DATA
        vm_ctx.registers[PC] = loaded.header.data_size + loaded.header.entry_point;
        
        return true;
    }

private:
    std::vector<uint8_t> m_ram;  // RAM storage
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