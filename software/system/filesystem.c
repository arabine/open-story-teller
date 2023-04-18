#include <ff.h>
#include "debug.h"
#include "ost_hal.h"

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
