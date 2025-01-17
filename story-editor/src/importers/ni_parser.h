#ifndef NI_PARSER_H
#define NI_PARSER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif


#define XI_BLOCK_SIZE   (8*1024)

typedef struct {
    uint32_t image_asset_index_in_ri;
    uint32_t sound_asset_index_in_si;
    uint32_t ok_btn_node_idx_in_li;
    uint32_t ok_btn_size_or_base_idx;
    uint32_t ok_btn_offset_from_base;
    uint32_t home_transition_action_node_index_in_li;
    uint32_t home_transition_number_of_options;
    uint32_t home_transition_selected_option_index;
    bool wheel;
    bool ok;
    bool home;
    bool pause;
    bool auto_play;
    uint16_t unknown;
} ni_node_t;

typedef struct {
    ni_node_t *current;
    char ri_file[13]; // 12 + \0
    char si_file[13]; // 12 + \0
} node_info_t;

typedef struct
{
    uint16_t ni_version;
    uint16_t pack_version;
    uint32_t node_list_start;
    uint32_t node_size;
    uint32_t stage_nodes_count;
    uint32_t image_assets_count;
    uint32_t sound_assets_count;

} ni_file_t;


uint32_t ni_get_number_of_images();
void ni_get_image(char buffer[13], uint32_t index);
uint32_t ni_get_node_index_in_li(uint32_t index_in_li, uint32_t selected);

// Block size at least 512 bytes be careful
void ni_decode_block512(uint8_t *data);

// block of any size
void ni_decode_block(uint8_t *data, uint32_t size);
bool ni_get_node_info(uint32_t index, node_info_t *node);

bool ni_parser(ni_file_t *ni_file, const uint8_t *data);
void ni_dump(ni_file_t *ni_file);

void ni_set_ri_block(const uint8_t *data, uint32_t size);
void ni_set_si_block(const uint8_t *data, uint32_t size);
void ni_set_li_block(const uint8_t *data, uint32_t size);

uint32_t ni_get_ri_block(uint8_t *data);
uint32_t ni_get_si_block(uint8_t *data);
uint32_t ni_get_li_block(uint8_t *data);

#ifdef	__cplusplus
}
#endif

#endif // NI_PARSER_H
