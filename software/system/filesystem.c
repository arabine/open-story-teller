#include <stdbool.h>
#include <ff.h>
#include "debug.h"
#include "ost_hal.h"

#include "filesystem.h"
#include "mini_qoi.h"

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

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 2

static FATFS fs;
static FIL File[2]; /* File object */
static DIR Dir;     /* Directory object */
static FILINFO Finfo;

static FRESULT scan_files(
    char *path /* Pointer to the working buffer with start path */
)
{
    DIR dirs;
    FRESULT fr;

    fr = f_opendir(&dirs, path);
    if (fr == FR_OK)
    {
        while (((fr = f_readdir(&dirs, &Finfo)) == FR_OK) && Finfo.fname[0])
        {
            if (Finfo.fattrib & AM_DIR)
            {

                //				i = strlen(path);
                debug_printf("/%s\r\n", Finfo.fname);
                if (fr != FR_OK)
                    break;
            }
            else
            {
                debug_printf("%s\r\n", Finfo.fname);
            }
        }
    }
    else
    {
        debug_printf("[OST] Cannot open directory\r\n");
    }

    return fr;
}

void filesystem_mount()
{
    uint32_t retries = 3;
    FRESULT fr;
    do
    {
        fr = f_mount(&fs, "", 1); // 0: mount successful ; 1: mount failed
        ost_system_delay_ms(10);
    } while (--retries && fr);

    if (fr)
    {
        debug_printf("[OST] No Card Found! Err=%d\r\n", (int)fr);
    }

    debug_printf("[OST] SD Card File System = %d\r\n", fs.fs_type); // FS_EXFAT = 4
    // debug_printf("[OST] Starting with CPU=%d\r\n", (int)SystemCoreClock);

    scan_files("");
}

// Ouvre le fichier d'index d'histoires
// Le format est le suivant :
//  - Fichier texte, avec fin de ligne \n
//  - Une ligne par histoire
//  - la ligne contient le nom du répertoire au format UUIDv4 (36 caractères ASCII avec 4 tirets '-')

// Exemple: d0ad13e4-06de-4e00-877c-d922fdd37d13

int is_multiple_of_37(int nombre)
{
    int plusGrandMultiple = (nombre / 37) * 37;
    return (nombre - plusGrandMultiple) == 0;
}

/*
// Loop in all directories
void disk_start()
{
      // default is current directory
  const char* dpath = ".";
  if (argc) dpath = args;

  DIR dir;
  if ( FR_OK != f_opendir(&dir, dpath) )
  {
    printf("cannot access '%s': No such file or directory\r\n", dpath);
    return;
  }

  FILINFO fno;
  while( (f_readdir(&dir, &fno) == FR_OK) && (fno.fname[0] != 0) )
  {
    if ( fno.fname[0] != '.' ) // ignore . and .. entry
    {
      if ( fno.fattrib & AM_DIR )
      {
        // directory
        printf("/%s\r\n", fno.fname);
      }else
      {
        printf("%-40s", fno.fname);
        if (fno.fsize < 1024)
        {
          printf("%lu B\r\n", fno.fsize);
        }else
        {
          printf("%lu KB\r\n", fno.fsize / 1024);
        }
      }
    }
  }

  f_closedir(&dir);
}
*/

bool filesystem_read_index_file(ost_context_t *ctx)
{
    FILINFO fno;
    FRESULT fr = f_stat("index.ost", &fno);

    ctx->number_of_stories = 0;
    ctx->current_story = 0;

    if (fr == FR_OK)
    {
        bool valid_file = false;
        int size = fno.fsize;
        // une ligne = 36 octets (UUID) + 1 octet (\n) = 37
        // Déjà, la taille doit être multiple de 37
        if (is_multiple_of_37(size) && (size > 0))
        {
            valid_file = true;
            ctx->number_of_stories = size / 37;
            debug_printf("SUCCESS: found %d stories\r\n", ctx->number_of_stories);
        }
    }
    else
    {
        debug_printf("ERROR: index.ost not found\r\n");
    }
}

static mqoi_dec_t dec;
static mqoi_desc_t desc;

static uint8_t bmpImage[256];

static color_t line[320];

file_t file_open(const char *filename)
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

void filesystem_display_image(const char *filename)
{
    file_t fil;
    unsigned int br;

    fil = file_open(filename);
    uint32_t imgW, imgH;

    mqoi_desc_init(&desc);

    // 1. Read header
    f_read(&fil, &desc.magic[0], sizeof(mqoi_desc_t) - 1, &br);

    uint8_t errn = mqoi_desc_verify(&desc, &imgW, &imgH);

    if (errn)
    {
        debug_printf("Invalid image, code %d\n", errn);
        return;
    }

    debug_printf("Image dimensions: %d, %d\n", imgW, imgH);

    uint32_t start, end, pxCount = 0;

    volatile mqoi_rgba_t *px;

    mqoi_dec_init(&dec, imgW * imgH);

    // Serial.println("starting decode...");
    int index = 256; // force refill first time
    int x = 0;
    int y = 0;
    while (!mqoi_dec_done(&dec))
    {
        if (index >= sizeof(bmpImage))
        {
            // refill buffer
            f_read(&fil, bmpImage, sizeof(bmpImage), &br);
            index = 0;
        }

        mqoi_dec_push(&dec, bmpImage[index++]);

        while ((px = mqoi_dec_pop(&dec)) != NULL)
        {
            pxCount++;

            line[x].r = px->r;
            line[x].g = px->g;
            line[x].b = px->b;

            x++;
            if (x >= 320)
            {
                ost_display_draw_h_line_rgb888(y, line);
                x = 0;
                y++;
            }
        }
    }

    f_close(&fil);
}

#define FS_MAX_SIZE_READ 256

void filesystem_load_rom(uint8_t *mem, const char *filename)
{
    file_t fil;
    FILINFO fno;
    unsigned int br;

    FRESULT fr = f_stat(filename, &fno);

    if (fr == FR_OK)
    {
        int total_size = fno.fsize;
        fil = file_open(filename);
        int copied_size = 0;

        do
        {
            int read_size = total_size > FS_MAX_SIZE_READ ? FS_MAX_SIZE_READ : total_size;

            f_read(&fil, &mem[copied_size], read_size, &br);

            if (br == read_size)
            {
                total_size -= read_size;
                copied_size += read_size;
            }
            else
            {
                debug_printf("Read file error\n");
                total_size = 0; // force exit
                return;
            }

        } while (total_size > 0);

        f_close(&fil);
    }
}
