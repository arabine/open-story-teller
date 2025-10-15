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
#include <iomanip>

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
    std::vector<uint8_t> ram;  // RAM storage
    std::vector<uint8_t> program;
    Chip32::Assembler assembler;
    chip32_binary_header_t header;
    
    
    Machine() {
        // Bind syscall handler to this instance
        m_syscallHandler = std::bind(&Machine::HandleSyscall, this, 
                                     std::placeholders::_1, 
                                     std::placeholders::_2);
        ram.resize(1024); 
    }


    bool BuildBinary(Assembler &assembler, std::vector<uint8_t> &program, chip32_binary_stats_t &stats)
    {
        program.clear();
        
        // ========================================================================
        // PHASE 1: Créer les sections temporaires
        // ========================================================================
        std::vector<uint8_t> constSection;     // DC - ROM constants only
        std::vector<uint8_t> codeSection;     // Executable code
        std::vector<uint8_t> initDataSection; // DV values + DZ zeros (RAM init)
        
        
        chip32_binary_header_init(&header);

        Chip32::Instr instr;
        if (assembler.GetMain(instr))
        {
            header.entry_point = instr.addr;
        }
        // else: no main found, we try to start at address zero...
        
        for (auto instr : assembler)
        {
            // ====================================================================
            // CODE INSTRUCTIONS
            // ====================================================================
            if (instr.isRomCode())
            {               
                // Ajouter l'opcode
                codeSection.push_back(instr.code.opcode);
                
                // Ajouter les arguments compilés
                std::copy(instr.compiledArgs.begin(), 
                        instr.compiledArgs.end(), 
                        std::back_inserter(codeSection));
            }
            // ====================================================================
            // ROM DATA - Distinguer DC (vraies constantes) de DV init data
            // ====================================================================
            else if (instr.isRomData && !instr.isRamData)
            {
                std::copy(instr.compiledArgs.begin(), 
                            instr.compiledArgs.end(), 
                            std::back_inserter(constSection));       
            }
            // ====================================================================
            // RAM VARIABLES
            // ====================================================================
            else if (instr.isRamData)
            {

                // ====================================================================
                // ZEROED DATA (DZ)
                // ====================================================================
                if (instr.isZeroData)
                {
                    header.bss_size += instr.dataLen;
                }
                // ====================================================================
                // INITIALIZED RAM VARIABLES (DV)
                // ====================================================================
                else
                {
                    // Ces données appartiennent à cette variable DV
                    initDataSection.insert(initDataSection.end(), 
                                    instr.compiledArgs.begin(), 
                                    instr.compiledArgs.end());
                }
            }
        }

        
        // ========================================================================
        // PHASE 5: Créer le header binaire
        // ========================================================================
        
        header.const_size = static_cast<uint32_t>(constSection.size());
        header.code_size = static_cast<uint32_t>(codeSection.size());
        header.data_size = static_cast<uint32_t>(initDataSection.size());
        
        if (initDataSection.size() > 0)
        {
            header.flags |= CHIP32_FLAG_HAS_INIT_DATA;
        }
        
        // ========================================================================
        // PHASE 6: Assembler le binaire final
        // ========================================================================
        uint32_t totalSize = chip32_binary_calculate_size(&header);
        program.resize(totalSize);
        
        uint32_t bytesWritten = chip32_binary_write(
            &header,
            constSection.empty() ? nullptr : constSection.data(),
            codeSection.empty() ? nullptr : codeSection.data(),
            initDataSection.empty() ? nullptr : initDataSection.data(),
            program.data(),
            static_cast<uint32_t>(program.size())
        );
        
        if (bytesWritten == 0 || bytesWritten != totalSize)
        {
            // Erreur lors de l'écriture
            program.clear();
            return false;
        }
        
        // ========================================================================
        // PHASE 7: Remplir les statistiques
        // ========================================================================
        chip32_binary_build_stats(&header, &stats);
        
        return true;
    }

    // ========================================================================
    // Méthode principale : Parse, Build, Execute
    // ========================================================================
    void QuickExecute(const std::string &assemblyCode)
    {
        chip32_binary_stats_t stats;

        // Parse
        parseResult = assembler.Parse(assemblyCode);
        
        if (!parseResult) {
            std::cout << "Parse error: " << assembler.GetLastError().ToString() << std::endl;
            return;
        }

        // Build binary with new format
        buildResult = BuildBinary(assembler, program, stats);
        chip32_binary_print_stats(&stats);
        
        if (!buildResult) {
            std::cout << "Build error: " << assembler.GetLastError().ToString() << std::endl;
            return;
        }

        // Load binary using executable format
        chip32_binary_error_t error = chip32_binary_load(
            &ctx,
            program.data(),
            static_cast<uint32_t>(program.size()),
            ram.data(),
            static_cast<uint32_t>(ram.size()),
            &header
        );
        
        if (error != CHIP32_BIN_OK) {
            std::cout << "Binary load error: " << chip32_binary_error_string(error) << std::endl;
            buildResult = false;
            return;
        }

        chip32_binary_build_stats(&header, &stats);
        chip32_binary_print_stats(&stats);
        
        // Set syscall handler using wrapper
        ctx.syscall = SyscallWrapper;
        ctx.user_data = this;

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


    // ========================================================================
    // Hexdump utilities
    // ========================================================================

    void HexDump(const uint8_t* data, uint32_t size, uint32_t base_addr = 0, const std::string& title = "")
    {
        if (!title.empty()) {
            std::cout << "\n=== " << title << " ===" << std::endl;
        }
        
        const int bytes_per_line = 16;
        
        for (uint32_t i = 0; i < size; i += bytes_per_line) {
            // Adresse
            std::cout << std::hex << std::setfill('0') << std::setw(8) 
                    << (base_addr + i) << ":  ";
            
            // Octets en hexadécimal
            for (int j = 0; j < bytes_per_line; j++) {
                if (i + j < size) {
                    std::cout << std::setw(2) << static_cast<int>(data[i + j]) << " ";
                } else {
                    std::cout << "   ";
                }
                
                // Séparateur au milieu
                if (j == 7) {
                    std::cout << " ";
                }
            }
            
            std::cout << " |";
            
            // Représentation ASCII
            for (int j = 0; j < bytes_per_line && i + j < size; j++) {
                uint8_t byte = data[i + j];
                if (byte >= 32 && byte <= 126) {
                    std::cout << static_cast<char>(byte);
                } else {
                    std::cout << '.';
                }
            }
            
            std::cout << "|" << std::dec << std::endl;
        }
        
        std::cout << std::endl;
    }

    void DumpRom()
    {
        if (ctx.rom.mem == nullptr || ctx.rom.size == 0) {
            std::cout << "ROM is empty or not loaded" << std::endl;
            return;
        }
        
        std::cout << "\n" << std::string(80, '=') << std::endl;
        std::cout << "ROM DUMP (Total: " << ctx.rom.size << " bytes)" << std::endl;
        std::cout << std::string(80, '=') << std::endl;
        
        uint32_t offset = 0;
        
        // Dump CONST section
        if (header.const_size > 0) {
            HexDump(ctx.rom.mem + offset, header.const_size, offset, 
                    "CONST Section (" + std::to_string(header.const_size) + " bytes)");
            offset += header.const_size;
        }
        
        // Dump CODE section
        if (header.code_size > 0) {
            HexDump(ctx.rom.mem + offset, header.code_size, offset,
                    "CODE Section (" + std::to_string(header.code_size) + " bytes)");
            offset += header.code_size;
        }
        
        // Dump INIT DATA section
        if (header.data_size > 0) {
            HexDump(ctx.rom.mem + offset, header.data_size, offset,
                    "INIT DATA Section (" + std::to_string(header.data_size) + " bytes)");
        }
    }

    void DumpRam()
    {
        if (ctx.ram.mem == nullptr || ctx.ram.size == 0) {
            std::cout << "RAM is empty or not allocated" << std::endl;
            return;
        }
        
        std::cout << "\n" << std::string(80, '=') << std::endl;
        std::cout << "RAM DUMP (Total: " << ctx.ram.size << " bytes)" << std::endl;
        std::cout << std::string(80, '=') << std::endl;
        
        uint32_t offset = 0;
        
        // Dump DATA section (initialized variables)
        if (header.data_size > 0) {
            HexDump(ctx.ram.mem + offset, header.data_size, 0x80000000 + offset,
                    "DATA Section (DV - " + std::to_string(header.data_size) + " bytes)");
            offset += header.data_size;
        }
        
        // Dump BSS section (zero-initialized variables)
        if (header.bss_size > 0) {
            HexDump(ctx.ram.mem + offset, header.bss_size, 0x80000000 + offset,
                    "BSS Section (DZ - " + std::to_string(header.bss_size) + " bytes)");
            offset += header.bss_size;
        }
        
        // Dump remaining RAM (heap/stack area)
        uint32_t remaining = ctx.ram.size - offset;
        if (remaining > 0) {
            // Limiter l'affichage à 256 bytes pour le reste
            uint32_t display_size = std::min(remaining, 256u);
            HexDump(ctx.ram.mem + offset, display_size, 0x80000000 + offset,
                    "Heap/Stack area (showing first " + std::to_string(display_size) + 
                    " of " + std::to_string(remaining) + " bytes)");
            
            if (remaining > display_size) {
                std::cout << "... (" << (remaining - display_size) 
                        << " more bytes not shown)" << std::endl;
            }
        }
    }

    void DumpMemory()
    {
        DumpRom();
        DumpRam();
    }

    // Variante pour dumper une région spécifique de RAM
    void DumpRamRegion(uint32_t start_addr, uint32_t size)
    {
        // Enlever le bit RAM si présent
        uint32_t addr = start_addr & 0x7FFFFFFF;
        
        if (ctx.ram.mem == nullptr || ctx.ram.size == 0) {
            std::cout << "RAM is not allocated" << std::endl;
            return;
        }
        
        if (addr >= ctx.ram.size) {
            std::cout << "Start address 0x" << std::hex << addr 
                    << " is out of RAM bounds (size: 0x" << ctx.ram.size 
                    << ")" << std::dec << std::endl;
            return;
        }
        
        uint32_t actual_size = std::min(size, ctx.ram.size - addr);
        
        std::cout << "\n" << std::string(80, '=') << std::endl;
        std::cout << "RAM Region Dump" << std::endl;
        std::cout << "Address: 0x" << std::hex << (0x80000000 | addr) 
                << " - 0x" << (0x80000000 | (addr + actual_size - 1)) 
                << std::dec << std::endl;
        std::cout << std::string(80, '=') << std::endl;
        
        HexDump(ctx.ram.mem + addr, actual_size, 0x80000000 | addr, "");
    }


private:
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