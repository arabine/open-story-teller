/*------------------------------------------------------------------------*/
/* STM32F100: MMCv3/SDv1/SDv2 (SPI mode) control module                   */
/*------------------------------------------------------------------------*/
/*
/  Copyright (C) 2018, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
   Module Private Functions
---------------------------------------------------------------------------*/

#include "ost_hal.h"
#include "debug.h"

#include "ff.h"
#include "diskio.h"
#include "sdcard.h"
#include "ffconf.h"

/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize(
	BYTE drv /* Physical drive number (0..) */
)
{
	if (drv)
		return STA_NOINIT;

	if (sdcard_init() != SD_RESPONSE_NO_ERROR)
		return STA_NOINIT;

	return 0;
}

/*-----------------------------------------------------------------------*/
/* Return Drive Status                                                   */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status(
	BYTE drv /* Physical drive number (0..) */
)
{
	if (drv)
		return STA_NOINIT;
	return 0;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT disk_read(
	BYTE pdrv,	  /* Physical drive nmuber to identify the drive */
	BYTE *buff,	  /* Data buffer to store read data */
	LBA_t sector, /* Start sector in LBA */
	UINT count	  /* Number of sectors to read */
)
{
	DRESULT res;
	if (pdrv || !count)
		return RES_PARERR;

	if (count == 1)
		res = (DRESULT)sdcard_sector_read(sector, buff);
	else
		res = (DRESULT)sdcard_sectors_read(sector, buff, count);

	if (res == 0x00)
		return RES_OK;
	return RES_ERROR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
#if _READONLY == 0
DRESULT disk_write(
	BYTE pdrv,		  /* Physical drive nmuber to identify the drive */
	const BYTE *buff, /* Data to be written */
	LBA_t sector,	  /* Start sector in LBA */
	UINT count		  /* Number of sectors to write */
)
{
	DRESULT res;
	if (pdrv || !count)
		return RES_PARERR;

	if (count == 1)
		res = (DRESULT)sdcard_sector_write(sector, buff);
	else
		res = (DRESULT)sdcard_sectors_write(sector, buff, count);

	if (res == 0)
		return RES_OK;
	return RES_ERROR;
}
#endif /* _READONLY */

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
	BYTE drv,  /* Physical drive number (0..) */
	BYTE ctrl, /* Control code */
	void *buff /* Buffer to send/receive control data */
)
{
	SD_CardInfo cardinfo;
	DRESULT res;
	if (drv)
		return RES_PARERR;

	switch (ctrl)
	{
	case CTRL_SYNC:
		//		res = ( SD_WaitReady() == SD_RESPONSE_NO_ERROR ) ? RES_OK : RES_ERROR
		res = RES_OK;
		break;
	case GET_BLOCK_SIZE:
		*(WORD *)buff = FF_MAX_SS;
		res = RES_OK;
		break;
	case GET_SECTOR_COUNT:
		if (sdcard_get_card_info(&cardinfo) != SD_RESPONSE_NO_ERROR)
			res = RES_ERROR;
		else
		{
			*(DWORD *)buff = cardinfo.CardCapacity;
			res = (*(DWORD *)buff > 0) ? RES_OK : RES_PARERR;
		}
		break;
	case CTRL_TRIM:
		res = (sdcard_sectors_erase(((DWORD *)buff)[0], ((DWORD *)buff)[1]) == SD_RESPONSE_NO_ERROR)
				  ? RES_OK
				  : RES_PARERR;
		break;
	default:
		res = RES_PARERR;
		break;
	}

	return res;
}
