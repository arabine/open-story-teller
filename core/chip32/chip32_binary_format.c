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
    uint8_t* binary,
    uint32_t size,
    chip32_loaded_binary_t* out_loaded
)
{
    if (!binary || !out_loaded) {
        return CHIP32_BIN_ERR_NULL_POINTER;
    }
    
    // Clear output structure
    memset(out_loaded, 0, sizeof(chip32_loaded_binary_t));
    
    // Check minimum size
    if (size < sizeof(chip32_binary_header_t)) {
        out_loaded->error = CHIP32_BIN_ERR_TOO_SMALL;
        return CHIP32_BIN_ERR_TOO_SMALL;
    }
    
    // Copy header
    memcpy(&out_loaded->header, binary, sizeof(chip32_binary_header_t));
    
    // Verify magic number
    if (out_loaded->header.magic != CHIP32_MAGIC) {
        out_loaded->error = CHIP32_BIN_ERR_INVALID_MAGIC;
        return CHIP32_BIN_ERR_INVALID_MAGIC;
    }
    
    // Check version
    if (out_loaded->header.version > CHIP32_VERSION) {
        out_loaded->error = CHIP32_BIN_ERR_UNSUPPORTED_VERSION;
        return CHIP32_BIN_ERR_UNSUPPORTED_VERSION;
    }
    
    // Calculate expected size
    uint32_t expected_size = sizeof(chip32_binary_header_t) +
                            out_loaded->header.data_size +
                            out_loaded->header.code_size +
                            out_loaded->header.init_data_size;
    
    if (size != expected_size) {
        out_loaded->error = CHIP32_BIN_ERR_SIZE_MISMATCH;
        return CHIP32_BIN_ERR_SIZE_MISMATCH;
    }
    
    // Set section pointers
    uint32_t offset = sizeof(chip32_binary_header_t);
    
    if (out_loaded->header.data_size > 0) {
        out_loaded->data_section = binary + offset;
        offset += out_loaded->header.data_size;
    }
    
    if (out_loaded->header.code_size > 0) {
        out_loaded->code_section = binary + offset;
        offset += out_loaded->header.code_size;
    }
    
    if (out_loaded->header.init_data_size > 0) {
        out_loaded->init_data_section = binary + offset;
    }
    
    out_loaded->error = CHIP32_BIN_OK;
    return CHIP32_BIN_OK;
}

void chip32_binary_get_stats(
    const chip32_loaded_binary_t* loaded,
    chip32_binary_stats_t* out_stats
)
{
    if (!loaded || !out_stats) {
        return;
    }
    
    out_stats->data_size = loaded->header.data_size;
    out_stats->bss_size = loaded->header.bss_size;
    out_stats->code_size = loaded->header.code_size;
    out_stats->init_data_size = loaded->header.init_data_size;
    
    out_stats->total_file_size = sizeof(chip32_binary_header_t) +
                                 loaded->header.data_size +
                                 loaded->header.code_size +
                                 loaded->header.init_data_size;
    
    out_stats->total_rom_size = loaded->header.data_size + 
                               loaded->header.code_size;
    
    out_stats->total_ram_size = loaded->header.bss_size;
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
           header->data_size +
           header->code_size +
           header->init_data_size;
}

uint32_t chip32_binary_write(
    const chip32_binary_header_t* header,
    const uint8_t* data_section,
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
    
    uint32_t offset = 0;
    
    // Write header
    memcpy(out_buffer + offset, header, sizeof(chip32_binary_header_t));
    offset += sizeof(chip32_binary_header_t);
    
    // Write DATA section
    if (header->data_size > 0) {
        if (!data_section) {
            return 0; // Data expected but NULL pointer
        }
        memcpy(out_buffer + offset, data_section, header->data_size);
        offset += header->data_size;
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
    if (header->init_data_size > 0) {
        if (!init_data_section) {
            return 0; // Init data expected but NULL pointer
        }
        memcpy(out_buffer + offset, init_data_section, header->init_data_size);
        offset += header->init_data_size;
    }
    
    return offset;
}

// ============================================================================
// RAM INITIALIZATION HELPER
// ============================================================================

uint32_t chip32_binary_init_ram(
    const chip32_loaded_binary_t* loaded,
    uint8_t* ram_buffer,
    uint32_t ram_size
)
{
    if (!loaded || !ram_buffer) {
        return 0;
    }
    
    // Check if binary has init data
    if (loaded->header.init_data_size == 0 || !loaded->init_data_section) {
        return 0;
    }
    
    // Copy init data to RAM (respect buffer limits)
    uint32_t copy_size = loaded->header.init_data_size;
    if (copy_size > ram_size) {
        copy_size = ram_size; // Truncate if RAM is smaller
    }
    
    memcpy(ram_buffer, loaded->init_data_section, copy_size);
    
    return copy_size;
}

// ============================================================================
// DEBUG/UTILITY FUNCTIONS
// ============================================================================

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
    printf("DATA section:   %u bytes (ROM constants)\n", header->data_size);
    printf("BSS section:    %u bytes (Total RAM: DV+DZ)\n", header->bss_size);
    printf("CODE section:   %u bytes\n", header->code_size);
    printf("Entry point:    0x%08X\n", header->entry_point);
    printf("Init data:      %u bytes (DV values + DZ zeros)\n", header->init_data_size);
    printf("\n");
}

void chip32_binary_print_stats(const chip32_binary_stats_t* stats)
{
    if (!stats) {
        return;
    }
    
    printf("=== Chip32 Binary Statistics ===\n");
    printf("DATA section:   %u bytes (ROM, initialized)\n", stats->data_size);
    printf("BSS section:    %u bytes (RAM, DV+DZ)\n", stats->bss_size);
    printf("CODE section:   %u bytes (ROM, executable)\n", stats->code_size);
    printf("Init data:      %u bytes (RAM initialization)\n", stats->init_data_size);
    printf("---\n");
    printf("File size:      %u bytes\n", stats->total_file_size);
    printf("ROM usage:      %u bytes (DATA + CODE)\n", stats->total_rom_size);
    printf("RAM usage:      %u bytes (BSS)\n", stats->total_ram_size);
    printf("\n");
}