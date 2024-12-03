/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "SPI_SDcard.h"

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */
#define DEV_SPI_SD	3	/* Example: Map SPI MMC/SD card to physical drive 3 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = STA_NOINIT;
	//int result;

	switch (pdrv) {
	case DEV_RAM :
		//result = RAM_disk_status();

		// translate the reslut code here

		return stat;

	case DEV_MMC :
		//result = MMC_disk_status();

		// translate the reslut code here

		return stat;

	case DEV_USB :
		//result = USB_disk_status();

		// translate the reslut code here

		return stat;

	case DEV_SPI_SD :
		return RES_OK;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = STA_NOINIT;

	switch (pdrv) {
	case DEV_RAM :
		//stat = RAM_disk_initialize();

		// translate the reslut code here

		return stat;

	case DEV_MMC :
		//stat = MMC_disk_initialize();

		// translate the reslut code here

		return stat;

	case DEV_USB :
		//stat = USB_disk_initialize();

		// translate the reslut code here

		return stat;

	case DEV_SPI_SD:
		stat = (DRESULT)SPI_SD_Init();

		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res = RES_ERROR;
	static uint8_t errcount = 0;
	if (!count)
		return RES_PARERR;

	switch (pdrv) {
	case DEV_RAM :
		// translate the arguments here

		//res = RAM_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_MMC :
		// translate the arguments here

		//res = MMC_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_USB :
		// translate the arguments here

		//res = USB_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_SPI_SD:
		res = (DRESULT)SPI_SD_ReadDisk2(buff, sector, count);
		while(res) // ошибка чтения
		{
			if(++errcount > 10)
			{
				errcount = 0;
				break;
			}
			SPI_SD_Init(); // Повторно инициализируем SD-карту
			res = (DRESULT)SPI_SD_ReadDisk2(buff, sector, count);
		}
		errcount = 0;
		return res;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res = RES_ERROR;
	static uint8_t errcount = 0;
	if (!count)
		return RES_PARERR;

	switch (pdrv) {
	case DEV_RAM :
		// translate the arguments here

		//result = RAM_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_MMC :
		// translate the arguments here

		//result = MMC_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_USB :
		// translate the arguments here

		//result = USB_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;
	
	case DEV_SPI_SD:
		res = (DRESULT)SPI_SD_WriteDisk2((uint8_t*)buff, sector, count);
		while(res) // ошибка чтения
		{
			if(++errcount > 10)
			{
				errcount = 0;
				break;
			}
			SPI_SD_Init(); // Повторно инициализируем SD-карту
			res = (DRESULT)SPI_SD_WriteDisk2((uint8_t*)buff, sector, count);
		}
		errcount = 0;
		return res;
	}

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data, Укажите NULL, когда он не используется*/
)
{
	DRESULT res = RES_PARERR;
	//int result;

	switch (pdrv) {
		case DEV_RAM :

			// Process of the command for the RAM drive

			return res;

		case DEV_MMC :

			// Process of the command for the MMC/SD card

			return res;

		case DEV_USB :

			// Process of the command the USB drive

			return res;

		case DEV_SPI_SD:
			switch (cmd)
			{
			case CTRL_SYNC:
				res = RES_OK;
				break;
			case GET_SECTOR_SIZE:
				*(DWORD*)buff = 512;
				res = RES_OK;
				break;
			case GET_BLOCK_SIZE:
				*(WORD*)buff = SPI_SDcardInfo.CardBlockSize;
				res = RES_OK;
				break;
			case GET_SECTOR_COUNT:
				*(DWORD*)buff = SPI_SDcardInfo.CardCapacity/512;
				res = RES_OK;
				break;
			default:
				res = RES_PARERR;
				break;
			}
			return res;
		}

	return RES_PARERR;
}

/*
Возвращает время RTC, формат:
		7bits				4bits			5bits			5bits			6bits			5bits
		31~25				24~21			20~16			15~11			10~5			4~0
// Количество лет		 1–12 месяцев 	   1–31 день 	   0–23 часа 	   0–59 минут    0–29 секунд
	с 1980 года
*/
DWORD get_fattime (void)
{
	DWORD RTC_buf = 0x00000000;
	#if SYSTEM_RTC_ENABLE  // Чтобы использовать эту функцию, необходимо включить RTC
		RTC_TimeTypeDef RTC_RealTime;
		RTC_DateTypeDef	RTC_RealDate;
		HAL_RTC_GetTime(&RTC_Handler, &RTC_RealTime, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&RTC_Handler, &RTC_RealDate, RTC_FORMAT_BIN);

		RTC_buf |= ((DWORD)(RTC_RealDate.Year+1970-1980)) << 25;
		RTC_buf |= ((DWORD)(RTC_RealDate.Month)) << 21;
		RTC_buf |= ((DWORD)(RTC_RealDate.Date)) << 16;
		RTC_buf |= ((DWORD)(RTC_RealTime.Hours)) << 11;
		RTC_buf |= ((DWORD)(RTC_RealTime.Minutes)) << 5;
		RTC_buf |= (DWORD)(RTC_RealTime.Seconds > 29 ? 29 : RTC_RealTime.Seconds);
		return RTC_buf;
	#else
		RTC_buf |= ((DWORD)(2024 - 1980)) << 25;
		RTC_buf |= ((DWORD)(9)) << 21;
		RTC_buf |= ((DWORD)(16)) << 16;
		RTC_buf |= ((DWORD)(14)) << 11;
		RTC_buf |= ((DWORD)(55)) << 5;
		RTC_buf |= (DWORD)(3);
		return RTC_buf;
	#endif
}