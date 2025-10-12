/*
 * Chip32 Binary Format - Pure C Implementation
 * Compatible with embedded systems
 * 
 * Variable types:
 *   DC8, DC32  : ROM constants (initialized data in ROM)
 *   DV8, DV32  : RAM variables with initial value
 *   DZ8, DZ32  : RAM zero-initialized arrays/buffers
 * 
 * Examples:
 *   $romString    DC8  "Hello"        ; String in ROM
 *   $romValue     DC32 42             ; Constant in ROM
 *   $ramCounter   DV32 100            ; RAM variable initialized to 100
 *   $ramMessage   DV8  "Test"         ; RAM string initialized to "Test"
 *   $ramBuffer    DZ8  256            ; RAM buffer of 256 bytes (zeroed)
 *   $ramArray     DZ32 100            ; RAM array of 100 x 32-bit (zeroed)
 * 
 * Binary file structure:
 * 
 * [HEADER - 28 bytes]
 *   - Magic number: "C32\0" (4 bytes)
 *   - Version: uint16_t (2 bytes)
 *   - Flags: uint16_t (2 bytes)
 *   - Data section size: uint32_t (4 bytes)
 *   - BSS section size: uint32_t (4 bytes)
 *   - Code section size: uint32_t (4 bytes)
 *   - Entry point: uint32_t (4 bytes)
 *   - Init data size: uint32_t (4 bytes)
 * 
 * [DATA SECTION]
 *   - ROM constants (DC8, DC32, etc.)
 * 
 * [CODE SECTION]
 *   - Executable instructions
 * 
 * [INIT DATA SECTION]
 *   - Initial RAM values: DV values + zeros for DZ areas
 *   - Size = sum of all DV and DZ declarations
 *   - Layout matches RAM layout exactly
 * 
 * 
 
+--------------------------------------+
|          Chip32 Header               |  <-- Début du fichier
| (Magic, Versions, Sizes, Entry...)   |
+--------------------------------------+
|          CONST Section (DV)           |  <-- Données initialisées
| (Variables globales initialisées)    |      e.g., const int x = 5;
+--------------------------------------+
|          CODE Section                |  <-- Instructions du programme
| (Les opcodes et leurs opérandes)     |
+--------------------------------------+
|     DATA Section (Optional)          |  <-- Données pour l'initialisation de la RAM (copie de DV)
| (Contient les valeurs initiales pour la RAM)
+--------------------------------------+
 

 */

#ifndef CHIP32_BINARY_FORMAT_H
#define CHIP32_BINARY_FORMAT_H

#include <stdint.h>
#include <string.h>
#include "chip32_vm.h"

#ifdef __cplusplus
extern "C" {
#endif

// Magic number: "C32\0" in little-endian
#define CHIP32_MAGIC 0x00323343

// Current version
#define CHIP32_VERSION 1

// Flags
#define CHIP32_FLAG_HAS_INIT_DATA 0x0001  // Binary contains RAM init data

// Error codes
typedef enum {
    CHIP32_BIN_OK = 0,
    CHIP32_BIN_ERR_TOO_SMALL,
    CHIP32_BIN_ERR_INVALID_MAGIC,
    CHIP32_BIN_ERR_UNSUPPORTED_VERSION,
    CHIP32_BIN_ERR_SIZE_MISMATCH,
    CHIP32_BIN_ERR_NULL_POINTER
} chip32_binary_error_t;

// Binary header structure (28 bytes, packed)
#pragma pack(push, 1)
typedef struct {
    uint32_t magic;              // Magic number "C32\0"
    uint16_t version;            // Format version
    uint16_t flags;              // Feature flags
    uint32_t const_size;          // Size of DATA section in bytes (ROM constants)
    uint32_t bss_size;           // Total RAM size (DV + DZ)
    uint32_t code_size;          // Size of CODE section in bytes
    uint32_t entry_point;        // Entry point offset in CODE section
    uint32_t init_data_size;     // Size of INIT DATA section (DV values + DZ zeros)
} chip32_binary_header_t;
#pragma pack(pop)

// Statistics
typedef struct {
    uint32_t const_size;          // ROM constants
    uint32_t bss_size;           // Total RAM needed
    uint32_t code_size;          // Executable code
    uint32_t init_data_size;     // RAM initialization data
    uint32_t total_file_size;    // File size on disk
    uint32_t total_rom_size;     // DATA + CODE
    uint32_t total_ram_size;     // RAM needed
} chip32_binary_stats_t;

// ============================================================================
// LOADING FUNCTIONS
// ============================================================================

/**
 * Load and validate a Chip32 binary
 * @param binary Pointer to binary data
 * @param size Size of binary data in bytes
 * @param out_loaded Output structure (filled on success)
 * @return Error code
 */
chip32_binary_error_t chip32_binary_load(
    chip32_ctx_t *ctx,
    uint8_t* binary,
    uint32_t binary_size,
    uint8_t* ram,
    uint32_t ram_size,
    chip32_binary_stats_t *out_stats
);


/**
 * Get error string from error code
 * @param error Error code
 * @return Human-readable error message
 */
const char* chip32_binary_error_string(chip32_binary_error_t error);

// ============================================================================
// BUILDING FUNCTIONS (for assembler/compiler)
// ============================================================================

/**
 * Initialize a binary header
 * @param header Header to initialize
 */
void chip32_binary_header_init(chip32_binary_header_t* header);

/**
 * Calculate total binary size from header
 * @param header Binary header
 * @return Total size in bytes
 */
uint32_t chip32_binary_calculate_size(const chip32_binary_header_t* header);

/**
 * Write a complete binary to memory
 * @param header Binary header
 * @param data_section DATA section content (can be NULL if const_size is 0)
 * @param code_section CODE section content (can be NULL if code_size is 0)
 * @param init_data_section INIT DATA section (can be NULL if init_data_size is 0)
 * @param out_buffer Output buffer (must be large enough)
 * @param buffer_size Size of output buffer
 * @return Number of bytes written, or 0 on error
 */
uint32_t chip32_binary_write(
    const chip32_binary_header_t* header,
    const uint8_t* data_section,
    const uint8_t* code_section,
    const uint8_t* init_data_section,
    uint8_t* out_buffer,
    uint32_t buffer_size
);

// ============================================================================
// DEBUG/UTILITY FUNCTIONS
// ============================================================================

/**
 * Print binary header information
 * @param header Binary header
 */
void chip32_binary_print_header(const chip32_binary_header_t* header);

/**
 * Print binary statistics
 * @param stats Binary statistics
 */
void chip32_binary_print_stats(const chip32_binary_stats_t* stats);

#ifdef __cplusplus
}
#endif

#endif // CHIP32_BINARY_FORMAT_H