#include "main.h"
#include <string.h>
#include "fatfs_add.h"
#include "fatfs_file_handler.h"
//#include "ff.h"
#include "sys_log.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

FATFS sd_fatfs;
char sd_path[4] = "3:";

FATFS *getfreefs;   // Read information
FRESULT fres;		// Common result code
extern FILINFO fno; 		// Structure holds information
DIR dir;			// Directory object structure
FIL sd_file;
extern SemaphoreHandle_t sd_mutex;
DWORD free_clusters;
DWORD free_sectors;
DWORD total_sectors;

MKFS_PARM mkfs_params[2] = {
	{FM_FAT32, 0, 0, 0, 0},
	{FM_FAT32, 0, 0, 0, 0}
};

StoreDisk_t sDisk[2] = {
	{sd_path, &sd_fatfs, &sd_file, &mkfs_params[0], 0 },
	{sd_path, &sd_fatfs, &sd_file, &mkfs_params[0], 0 }
};

void dmesg(FRESULT res) {

	switch (res) {
	case FR_OK:
		log_i(FAT_TAG, "Succeeded \n");
		break;
	case FR_DISK_ERR:
		log_i(FAT_TAG, "A hard error occurred in the low level disk I/O layer \n");
		break;
	case FR_INT_ERR:
		log_i(FAT_TAG, "Assertion failed \n");
		break;
	case FR_NOT_READY:
		log_i(FAT_TAG, "The physical drive cannot work");
		break;
	case FR_NO_FILE:
		log_i(FAT_TAG, "Could not find the file \n");
		break;
	case FR_NO_PATH:
		log_i(FAT_TAG, "Could not find the path \n");
		break;
	case FR_INVALID_NAME:
		log_i(FAT_TAG, "The path name format is invalid \n");
		break;
	case FR_DENIED:
		log_i(FAT_TAG, "Access denied due to prohibited access or directory full \n");
		break;
	case FR_EXIST:
		log_i(FAT_TAG, "Exist or access denied due to prohibited access \n");
		break;
	case FR_INVALID_OBJECT:
		log_i(FAT_TAG, "The file/directory object is invalid \n");
		break;
	case FR_WRITE_PROTECTED:
		log_i(FAT_TAG, "The physical drive is write protected \n");
		break;
	case FR_INVALID_DRIVE:
		log_i(FAT_TAG, "The logical drive number is invalid \n");
		break;
	case FR_NOT_ENABLED:
		log_i(FAT_TAG, "The volume has no work area");
		break;
	case FR_NO_FILESYSTEM:
		log_i(FAT_TAG, "There is no valid FAT volume");
		break;
	case FR_MKFS_ABORTED:
		log_i(FAT_TAG, "The f_mkfs() aborted due to any parameter error \n");
		break;
	case FR_TIMEOUT:
		log_i(FAT_TAG,
				"Could not get a grant to access the volume within defined period \n");
		break;
	case FR_LOCKED:
		log_i(FAT_TAG,
				"The operation is rejected according to the file sharing policy \n");
		break;
	case FR_NOT_ENOUGH_CORE:
		log_i(FAT_TAG, "LFN working buffer could not be allocated \n");
		break;
	case FR_TOO_MANY_OPEN_FILES:
		log_i(FAT_TAG, "Number of open files > _FS_SHARE \n");
		break;
	case FR_INVALID_PARAMETER:
		log_i(FAT_TAG, "Given parameter is invalid \n");
		break;
	default:
		log_i(FAT_TAG, "An error occured. (%d)\n", fres);
	}

}

void ls(char *path)
{
	log_i(FAT_TAG, "Files/Folder List:");
	fres = f_opendir(&dir, path);
	if (fres == FR_OK)
	{
		while (1)
		{
			fres = f_readdir(&dir, &fno);
			if ((fres != FR_OK) || (fno.fname[0] == 0))
				break;
			log_i(FAT_TAG, "%c%c%c%c%c %u-%02u-%02u, %02u:%02u %10d %s/%s",
							((fno.fattrib & AM_DIR) ? 'D' : '-'),
							((fno.fattrib & AM_RDO) ? 'R' : '-'),
							((fno.fattrib & AM_SYS) ? 'S' : '-'),
							((fno.fattrib & AM_HID) ? 'H' : '-'),
							((fno.fattrib & AM_ARC) ? 'A' : '-'),
							((fno.fdate >> 9) + 1980), (fno.fdate >> 5 & 15),
							(fno.fdate & 31), (fno.ftime >> 11), (fno.ftime >> 5 & 63),
							(int) fno.fsize, path, fno.fname);
		}
		f_closedir(&dir);
	}
	else log_i(FAT_TAG, "Failed to open \"%s\". (%u)\n", path, fres);

}

void sdinfo_task(void *params)
{
	StoreDisk_t *disk_ptr = (StoreDisk_t*) params;
    while (1)
    {
        log_i(FAT_TAG, "\tStartSDinfoTask!\n");
        xSemaphoreTake(sd_mutex, portMAX_DELAY);
        log_i(FAT_TAG, "\t[SDinfo]: Mount SDCard");
		FH_mount(&disk_ptr[0]);
        //fres = f_mount(&sd_fatfs, "3:", 1);
        if ((&disk_ptr[0])->mount_cnt > 0)
        {
			FH_get_space(&disk_ptr[0]);
            // fres = f_getfree("3:", &free_clusters, &getfreefs);
            // total_sectors = (getfreefs->n_fatent - 2) * getfreefs->csize;
            // free_sectors = free_clusters * getfreefs->csize;
            // log_i(FAT_TAG, "\n[SDinfo]: SD Card Status:\r\n\n%10lu KiB total drive space.\r\n%10lu KiB available.\r\n", total_sectors / 2, free_sectors / 2);
			// vTaskDelay(pdMS_TO_TICKS(100));
			// // List files and folder in root
			// log_i(FAT_TAG, "\n[SDinfo]: ");
			// ls("3:");
			FH_scan(&disk_ptr[0], "", SCAN_SHOWTITLE | SCAN_RECURSIVE);
        }
		// else
		// {
		// 	log_i(FAT_TAG, "\n[SDinfo]: ");
		// 	dmesg(fres);
		// }
		FH_unmount(&disk_ptr[0]);
		//f_mount(NULL, "3:", 0);
		log_i(FAT_TAG, "\t[SDinfo]: Unmount SDCard");
		xSemaphoreGive(sd_mutex);
		vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

void sdmanager_task(void *params)
{
	while (1)
	{
		log_i(FAT_TAG, "\nStartSDmanagerTask!");
		xSemaphoreTake(sd_mutex, portMAX_DELAY);
		log_i(FAT_TAG, "\n[SDmanager]: Mount SDCard");
		fres = f_mount(&sd_fatfs, "3:", 1);
		if (fres == FR_OK)
		{
			fres = f_mkdir("3:DEMO");
			if (fres == FR_OK)
			{
				log_i(FAT_TAG, "\n[SDmanager] Create folder");
			}
			else
			{
				log_i(FAT_TAG, "\n[SDmanager][Create folder]: ");
				dmesg(fres);				
			}

			TCHAR readbuf[30];

			// Read File
			fres = f_open(&sd_file, "3:test.txt", FA_OPEN_ALWAYS);
			if (fres == FR_OK)
			{
				TCHAR *rres = f_gets(readbuf, sizeof(readbuf), &sd_file);
				if (rres != 0)
				{
					log_i(FAT_TAG, "\n[SDManager]: Read File");
					log_i(FAT_TAG, "\n[SDManager]: 'test.txt' contents: %s",
									readbuf);
				}
			}
			else
			{
				log_i(FAT_TAG, "\n[SDmanager][Read file]: ");
				dmesg(fres);
			}
			f_close(&sd_file);

			vTaskDelay(pdMS_TO_TICKS(500));

			// Write File
			fres = f_open(&sd_file, "3:/DEMO/write.txt", FA_WRITE | FA_OPEN_ALWAYS | FA_CREATE_ALWAYS);
			if (fres == FR_OK)
			{
				strncpy(readbuf, "What the fuck it?", 18);
				UINT byteswrote;
				fres = f_write(&sd_file, readbuf, 17, &byteswrote);
				if (fres == FR_OK)
				{
					log_i(FAT_TAG, "\n[SDManager]: Write File");
					log_i(FAT_TAG, "Wrote %d bytes to 'write.txt'",
									byteswrote);
				}
				else
				{
					log_i(FAT_TAG, "\n[SDmanager][Write file]: ");
					dmesg(fres);
				}
			}
			else
			{
				log_i(FAT_TAG, "\n[SDManager]:[Create File]: ");
				dmesg(fres);
			}
			f_close(&sd_file);
		}
		else 
		{
			log_i(FAT_TAG, "\n[SDmanager]: ");
			dmesg(fres);
		}

		f_mount(NULL, "3:", 0);
		log_i(FAT_TAG, "\n[SDmanager]: Unmount SDCard");
		xSemaphoreGive(sd_mutex);
		vTaskDelay(pdMS_TO_TICKS(1500));
	}
}