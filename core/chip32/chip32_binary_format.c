/*
 * Chip32 Binary Format - Implementation
 */

#include "chip32_binary_format.h"
#include <stdio.h>

// Verify header size at compile time
_Static_assert(sizeof(chip32_binary_header_t) == 28, "Header must be 28 bytes");

// ============================================================================
// LOADING FUNCTIONS
// ============================================================================

chip32_binary_error_t chip32_binary_load(
    chip32_ctx_t *ctx,
    uint8_t* binary,
    uint32_t binary_size,
    uint8_t* ram,
    uint32_t ram_size,
    chip32_binary_header_t *header
)
{
    // Initialize VM
    chip32_initialize(ctx);
    ctx->stack_size = 512; // Can be changed outside this function

    // Clear RAM
    memset(ram, 0, ram_size);

    if (!binary || !ctx) {
        return CHIP32_BIN_ERR_NULL_POINTER;
    }

    // Check minimum size
    if (binary_size < sizeof(chip32_binary_header_t)) {
        return CHIP32_BIN_ERR_TOO_SMALL;
    }
    
    // Copy header
    memcpy(header, binary, sizeof(chip32_binary_header_t));
    
    // Verify magic number
    if (header->magic != CHIP32_MAGIC) {
        return CHIP32_BIN_ERR_INVALID_MAGIC;
    }
    
    // Check version
    if (header->version > CHIP32_VERSION) {
        return CHIP32_BIN_ERR_UNSUPPORTED_VERSION;
    }
    
    // Calculate expected size
    uint32_t expected_size = sizeof(chip32_binary_header_t) +
                            header->const_size +
                            header->code_size +
                            header->data_size;
    
    if (binary_size != expected_size) {
        return CHIP32_BIN_ERR_SIZE_MISMATCH;
    }
    
    // Set section pointers
    uint32_t offset = sizeof(chip32_binary_header_t);

    // Skip header for ROM executable (must start at a valide code address)
    ctx->rom.mem = binary + offset;
    ctx->rom.size = binary_size - offset;
    ctx->rom.addr = 0;

    // RAM and ROM are in the same logical memory plane
    // So we set it begin after the ROM (why not)
    ctx->ram.mem = ram;
    ctx->ram.addr = ctx->rom.size;
    ctx->ram.size = ram_size;

    // Set entry point (DATA size + entry point offset in CODE)
    ctx->registers[PC] = header->entry_point;
    // Stack pointer at the end of the RAM
    ctx->registers[SP] = ctx->ram.size;

    // Load data initialized values
    const uint8_t *data = binary + offset + header->const_size + header->code_size;
    memcpy(ram, data, header->data_size);

    return CHIP32_BIN_OK;
}

const char* chip32_binary_error_string(chip32_binary_error_t error)
{
    switch (error) {
        case CHIP32_BIN_OK:
            return "No error";
        case CHIP32_BIN_ERR_TOO_SMALL:
            return "Binary too small (less than header size)";
        case CHIP32_BIN_ERR_INVALID_MAGIC:
            return "Invalid magic number (not a Chip32 binary)";
        case CHIP32_BIN_ERR_UNSUPPORTED_VERSION:
            return "Unsupported binary version";
        case CHIP32_BIN_ERR_SIZE_MISMATCH:
            return "Binary size mismatch (corrupted file?)";
        case CHIP32_BIN_ERR_NULL_POINTER:
            return "NULL pointer argument";
        default:
            return "Unknown error";
    }
}

// ============================================================================
// BUILDING FUNCTIONS
// ============================================================================

void chip32_binary_header_init(chip32_binary_header_t* header)
{
    if (!header) {
        return;
    }
    
    memset(header, 0, sizeof(chip32_binary_header_t));
    header->magic = CHIP32_MAGIC;
    header->version = CHIP32_VERSION;
    header->flags = 0;
}

uint32_t chip32_binary_calculate_size(const chip32_binary_header_t* header)
{
    if (!header) {
        return 0;
    }
    
    return sizeof(chip32_binary_header_t) +
           header->const_size +
           header->code_size +
           header->data_size;
}

uint32_t chip32_binary_write(
    const chip32_binary_header_t* header,
    const uint8_t* const_section,
    const uint8_t* code_section,
    const uint8_t* init_data_section,
    uint8_t* out_buffer,
    uint32_t buffer_size
)
{
    if (!header || !out_buffer) {
        return 0;
    }
    
    // Calculate required size
    uint32_t required_size = chip32_binary_calculate_size(header);
    
    if (buffer_size < required_size) {
        return 0; // Buffer too small
    }
    
    // Write header
    memcpy(out_buffer, header, sizeof(chip32_binary_header_t));
    uint32_t offset = sizeof(chip32_binary_header_t);
    
    // Write CONST section
    if (header->const_size > 0) {
        if (!const_section) {
            return 0; // Data expected but NULL pointer
        }
        memcpy(out_buffer + offset, const_section, header->const_size);
        offset += header->const_size;
    }
    
    // Write CODE section
    if (header->code_size > 0) {
        if (!code_section) {
            return 0; // Code expected but NULL pointer
        }
        memcpy(out_buffer + offset, code_section, header->code_size);
        offset += header->code_size;
    }
    
    // Write INIT DATA section
    if (header->data_size > 0) {
        if (!init_data_section) {
            return 0; // Init data expected but NULL pointer
        }
        memcpy(out_buffer + offset, init_data_section, header->data_size);
        offset += header->data_size;
    }
    
    return offset;
}


// ============================================================================
// DEBUG/UTILITY FUNCTIONS
// ============================================================================


void chip32_binary_build_stats(const chip32_binary_header_t *header, chip32_binary_stats_t* out_stats)
{
    out_stats->const_size = header->const_size;
    out_stats->bss_size = header->bss_size;
    out_stats->code_size = header->code_size;
    out_stats->data_size = header->data_size;
    
    out_stats->total_file_size = sizeof(chip32_binary_header_t) +
                                 header->const_size +
                                 header->code_size +
                                 header->data_size;
    
    out_stats->total_rom_size = header->const_size + 
                               header->code_size;
    
    out_stats->total_ram_size = header->bss_size;

}

void chip32_binary_print_header(const chip32_binary_header_t* header)
{
    if (!header) {
        return;
    }
    
    printf("=== Chip32 Binary Header ===\n");
    printf("Magic:          0x%08X", header->magic);
    if (header->magic == CHIP32_MAGIC) {
        printf(" (valid)\n");
    } else {
        printf(" (INVALID!)\n");
    }
    printf("Version:        %u\n", header->version);
    printf("Flags:          0x%04X", header->flags);
    if (header->flags & CHIP32_FLAG_HAS_INIT_DATA) {
        printf(" (has init data)");
    }
    printf("\n");
    printf("CONST section:   %u bytes (ROM constants)\n", header->const_size);
    printf("BSS section:    %u bytes (Total RAM: DV+DZ)\n", header->bss_size);
    printf("CODE section:   %u bytes\n", header->code_size);
    printf("Entry point:    0x%08X\n", header->entry_point);
    printf("Init data:      %u bytes (DV values + DZ zeros)\n", header->data_size);
    printf("\n");
}

void chip32_binary_print_stats(const chip32_binary_stats_t* stats)
{
    if (!stats) {
        return;
    }
    
    printf("=== Chip32 Binary Statistics ===\n");
    printf("CONST section:   %u bytes (ROM, initialized)\n", stats->const_size);
    printf("BSS section:    %u bytes (RAM, DZ)\n", stats->bss_size);
    printf("CODE section:   %u bytes (ROM, executable)\n", stats->code_size);
    printf("Init data:      %u bytes (RAM initialization)\n", stats->data_size);
    printf("---\n");
    printf("File size:      %u bytes\n", stats->total_file_size);
    printf("ROM usage:      %u bytes (DATA + CODE)\n", stats->total_rom_size);
    printf("RAM usage:      %u bytes (BSS)\n", stats->total_ram_size);
    printf("\n");
}
