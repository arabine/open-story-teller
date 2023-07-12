
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libutil.h"
#include "ost_hal.h"
#include "debug.h"

#ifdef OST_USE_FF_LIBRARY
#include "ff.h"
#include "diskio.h"
typedef FIL file_t;
#else

// Use standard library
typedef FILE *file_t;
typedef int FRESULT;
#define F_OK
#endif

file_t ost_file_open(const char *filename)
{
#ifdef OST_USE_FF_LIBRARY
    file_t fil;
    FRESULT fr = f_open(&fil, filename, FA_READ);
    if (fr != FR_OK)
    {
        debug_printf("ERROR: f_open %d\n\r", (int)fr);
    }
    return fil;
#else
    return fopen(filename, "r");
#endif
}

static uint8_t bmpImage[512];

static uint8_t decompressed[1024];
static uint8_t palette[16 * 4];

typedef struct
{
    uint16_t type; /* Magic identifier            */
    uint32_t size; /* File size in bytes          */
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset; /* Offset to image data, bytes */
} bmp_header_t;

typedef struct
{
    uint32_t size; /* Header size in bytes      */
    uint32_t width;
    uint32_t height;      /* Width and height of image */
    uint16_t planes;      /* Number of colour planes   */
    uint16_t bits;        /* Bits per pixel            */
    uint32_t compression; /* Compression type          */
    uint32_t imagesize;   /* Image size in bytes       */
    uint32_t xresolution;
    uint32_t yresolution;      /* Pixels per meter          */
    uint32_t ncolours;         /* Number of colours         */
    uint32_t importantcolours; /* Important colours         */
    uint32_t rgb;
    uint32_t rgb2;
} bmp_infoheader_t;

static const uint32_t HEADER_SIZE = 14;
static const uint32_t INFO_HEADER_SIZE = 40;

uint8_t parse_bmp(const uint8_t *data, bmp_header_t *header, bmp_infoheader_t *info_header)
{
    uint8_t isBmp = 0;

    // Header is 14 bytes length
    isBmp = (data[0] == 'B') && (data[1] == 'M') ? 1 : 0;
    header->size = leu32_get(data + 2);
    header->offset = leu32_get(data + 10);

    info_header->size = leu32_get(data + HEADER_SIZE);
    info_header->width = leu32_get(data + HEADER_SIZE + 4);
    info_header->height = leu32_get(data + HEADER_SIZE + 8);
    info_header->planes = leu16_get(data + HEADER_SIZE + 12);
    info_header->bits = leu16_get(data + HEADER_SIZE + 14);
    info_header->compression = leu32_get(data + HEADER_SIZE + 16);
    info_header->imagesize = leu32_get(data + HEADER_SIZE + 20);
    info_header->xresolution = leu32_get(data + HEADER_SIZE + 24);
    info_header->yresolution = leu32_get(data + HEADER_SIZE + 28);
    info_header->ncolours = leu32_get(data + HEADER_SIZE + 32);
    info_header->importantcolours = leu32_get(data + HEADER_SIZE + 36);
    info_header->rgb = leu32_get(data + HEADER_SIZE + 40);
    info_header->rgb2 = leu32_get(data + HEADER_SIZE + 44);

    return isBmp;
}

void picture_show(const char *filename)
{
    file_t fil;
    uint32_t offset;
    unsigned int br;

    fil = ost_file_open(filename); //"0_000_314CBAA1.bmp");
    offset = 0;

#ifdef OST_USE_FF_LIBRARY
    f_lseek(&fil, offset);
    f_read(&fil, bmpImage, 512, &br);
#else
    fseek(fil, offset, SEEK_SET);
    fread(bmpImage, sizeof(uint8_t), 512, fil);
#endif
    int nblines = 0;

    bmp_header_t header;
    bmp_infoheader_t info_header;
    parse_bmp(bmpImage, &header, &info_header);

    // Compute some sizes
    uint32_t fileSize = header.size;
    // uint32_t compressedSize = fileSize - header.offset;
    uint32_t paletteSize = header.offset - (HEADER_SIZE + INFO_HEADER_SIZE);

    // Copy palette
    offset = HEADER_SIZE + INFO_HEADER_SIZE;
    memcpy(palette, bmpImage + HEADER_SIZE + INFO_HEADER_SIZE, paletteSize);

    offset = header.offset;
    uint8_t *compressed = &bmpImage[header.offset]; // start with data just after the BPM header

    debug_printf("File size (from header):%d\r\n", (uint32_t)header.size);
    debug_printf("File size (from data):%d\r\n", (uint32_t)fileSize);
    debug_printf("Data offset:%d\r\n", (uint32_t)header.offset);
    debug_printf("Image size:%d\r\n", (uint32_t)info_header.size);
    debug_printf("width:%d\r\n", (uint32_t)info_header.width);
    debug_printf("height:%d\r\n", (uint32_t)info_header.height);
    debug_printf("Planes:%d\r\n", (uint32_t)info_header.planes);
    debug_printf("Bits:%d\r\n", (uint32_t)info_header.bits);
    debug_printf("Compression:%d\r\n", (uint32_t)info_header.compression); // 2 - 4 bit run length encoding
    debug_printf("Image size:%d\r\n", (uint32_t)info_header.imagesize);
    debug_printf("X resolution:%d\r\n", (uint32_t)info_header.xresolution);
    debug_printf("Y resolution:%d\r\n", (uint32_t)info_header.yresolution);
    debug_printf("Colors:%d\r\n", (uint32_t)info_header.ncolours);
    debug_printf("Important colors:%d\r\n", (uint32_t)info_header.importantcolours);
    debug_printf("RGB :%d\r\n", (uint32_t)info_header.rgb);
    debug_printf("RGB2 :%d\r\n", (uint32_t)info_header.rgb2);

    // buffer de sortie, bitmap décompressé
    memset(decompressed, 0, sizeof(decompressed));

    //  btea((uint32_t*) bmpImage, -128, key);

    uint32_t pixel = 0; // specify the pixel offset
    bool end = false;
    uint32_t i = 0;
    uint32_t totalPixels = 0;
    rect_t pos;
    pos.x = 0;
    pos.y = 0;
    pos.width = info_header.width;
    pos.height = 1;
    do
    {
        // if we are behond the middle of the buffer, read more data from file
        if (i > 256)
        {
            offset = offset + i;

#ifdef OST_USE_FF_LIBRARY
            f_lseek(&fil, offset);
            f_read(&fil, bmpImage, 512, &br);
#else
            fseek(fil, offset, SEEK_SET);
            fread(bmpImage, sizeof(uint8_t), 512, fil);
#endif
            compressed = &bmpImage[0];
            i = 0;
        }

        uint8_t rleCmd = compressed[i];
        if (rleCmd > 0)
        {
            uint8_t val = compressed[i + 1];
            // repeat number of pixels
            for (uint32_t j = 0; j < rleCmd; j++)
            {
                if ((j & 1) == 0)
                {
                    decompressed[pixel] = (val & 0xF0) >> 4;
                }
                else
                {
                    decompressed[pixel] = (val & 0x0F);
                }
                pixel++;

                if (pixel > info_header.width)
                {
                    debug_printf("!"); // should be an error here
                }
            }
            i += 2; // jump pair instruction
        }
        else
        {
            uint8_t second = compressed[i + 1];
            if (second == 0)
            {
                if (pixel % info_header.width)
                {
                    // end of line
                    uint32_t lines = pixel / info_header.width;
                    uint32_t remaining = info_header.width - (pixel - (lines * info_header.width));

                    pixel += remaining;
                }
                i += 2;
            }
            else if (second == 1)
            {
                end = true;
            }
            else if (second == 2)
            {
                // delta N pixels and M lines
                pixel += compressed[i + 2] + compressed[i + 3] * info_header.width;
                i += 4;
            }
            else
            {
                // absolute mode
                uint8_t *ptr = &compressed[i + 2];
                // repeat number of pixels
                for (uint32_t j = 0; j < second; j++)
                {
                    if ((j & 1) == 0)
                    {
                        decompressed[pixel] = (*ptr & 0xF0) >> 4;
                    }
                    else
                    {
                        decompressed[pixel] = (*ptr & 0x0F);
                        ptr++;
                    }
                    pixel++;

                    if (pixel >= info_header.width)
                    {
                        debug_printf("!");
                        //  pixel = 0;
                    }
                }
                i += 2 + (second / 2);

                // padded in word boundary, jump if necessary
                if ((second / 2) % 2)
                {
                    i++;
                }
            }

            if (pixel == info_header.width)
            {
                // enough pixels to write a line to the screen
                ost_display_draw_h_line(pos.y, decompressed, palette);
                // debug_printf("POS Y: %d", pos.y);

                memset(decompressed, 0, sizeof(decompressed));
                // ili9341_write(&pos, decompressed);
                // next line...
                pos.y++;
                totalPixels += info_header.width;
                pixel = 0;
                nblines++;
            }
            else if (pixel > info_header.width)
            {
                debug_printf("!");
                //  pixel = 0;00000000000000000000000000000000000000
            }
        }

        if (totalPixels > (info_header.width * info_header.height))
        {
            end = true; // error
        }
    } while ((offset < fileSize) && !end);

    // if (end)
    // {
    //     debug_printf("\r\n>>>>> Decoding error !! <<<<\r\n");
    // }

    // Fill missing lines
    if (nblines < info_header.height)
    {
        memset(decompressed, 0, sizeof(decompressed));
        int missing_lines = (info_header.height - nblines);
        for (int i = 0; i < missing_lines; i++)
        {
            // ili9341_draw_h_line(pos.y, decompressed, palette);
            nblines++;
        }
    }

#ifdef OST_USE_FF_LIBRARY
    f_close(&fil);
#else
    fclose(fil);
#endif

    debug_printf("\r\nNb lines :%d\r\nTotal pixels: %d", (uint32_t)nblines, (uint32_t)totalPixels);
}
