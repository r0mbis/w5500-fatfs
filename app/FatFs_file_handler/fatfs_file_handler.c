/**
 * @brief FATFS User API
 * 
 * @author	shadowthreed@gmail.com
 * @date	20240813
 */
#include "fatfs_file_handler.h"
#include "stm32f4xx_hal.h"
#include <stdlib.h>
#include <stdio.h>
#include "main.h"

#if SD_LOG_EN	
	#ifndef FH_DBG
	#define FH_DBG		log_d
	#endif
	
	#ifndef FH_ERR
	#define FH_ERR		log_e
	#endif
#else
	#define FH_DBG(fmt, ...)
	#define FH_ERR(fmt, ...)
#endif

#ifndef FH_printf
#define FH_printf		log_i
#endif

FILINFO fno;
FRESULT fresult;

void FH_mkfs(StoreDisk_t* disk)
{
	// Make sure the stact is enough!
	BYTE work[FF_MAX_SS];		// work size must not less than the disk sector size(must sector size is _MAX_SS)
	fresult = f_mkfs(disk->name, disk->opt, work, sizeof work);
	if(fresult != FR_OK) {
		FH_ERR(FAT_TAG, "make FS[%s] err[%d]\n", disk->name, fresult);
	}
}

void FH_mount(StoreDisk_t* disk)
{
	fresult = f_mount(disk->fatfs, disk->name, 1);
	if(fresult != FR_OK) {
		FH_ERR(FAT_TAG, "\tmount SD err[%d]\n", fresult);
		return;
	}
	disk->mount_cnt++;
}

void FH_unmount(StoreDisk_t* disk)
{
	if(disk->mount_cnt > 0) {
		disk->mount_cnt--;
		if (disk->mount_cnt > 0)
			return;
	}
	
	fresult = f_mount(NULL, disk->name, 0);
	if(fresult != FR_OK) {
		FH_ERR(FAT_TAG, "\tunmount SD err[%d]\n", fresult);
	}
	else disk->mount_cnt = 0;
}

/* Start node to be scanned (***also used as work area***) */
/**
 * flags: SCAN_SHOWTITLE, SCAN_RECURSIVE
 */
FRESULT FH_scan(StoreDisk_t* disk, char* scanPath, uint8_t flags)
{
	DIR dir;
	UINT i;
	char path[MAX_PATH_LEN*2];
	
	if(disk == NULL || disk->name == NULL) {
		FH_ERR(FAT_TAG, "\tparam err\n");
		return FR_INVALID_PARAMETER;
	}
	
	if(scanPath != NULL) {
		if(strlen(disk->name) + strlen(scanPath) >= MAX_PATH_LEN) {
			FH_ERR(FAT_TAG, "\tscan path too long\n");
			return FR_INVALID_PARAMETER;
		}
		sprintf(path, "%s%s", disk->name, scanPath);
	} else {
		sprintf(path, "%s", disk->name);
	}

	fresult = f_opendir(&dir, path);                       /* Open the directory */
	if(fresult == FR_OK) {
		if(flags & SCAN_SHOWTITLE) {
			FH_printf(FAT_TAG, "\tScan [%s]", path);
		}
		for(;;) {
			fresult = f_readdir(&dir, &fno);               /* Read a directory item */
			if(fresult != FR_OK || fno.fname[0] == 0) {
				break;  /* Break on error or end of dir */
			}
			if (fno.fattrib & AM_DIR) {     /* It is a directory */
				if(!(strcmp ("SYSTEM~1", fno.fname))) {
					continue;
				}
				FH_printf(FAT_TAG, "\t[Dir ] %s/%s", path, fno.fname);
				if(strlen(path) + 1 + strlen(fno.fname) >= MAX_PATH_LEN) {
					FH_ERR(FAT_TAG, "\tpath too long");
					break;
				}
				if (flags & SCAN_RECURSIVE)
				{
					i = strlen(path);
					sprintf(&path[i], "/%s", fno.fname);
					fresult = FH_scan(disk, &path[strlen(disk->name)], flags & (~SCAN_SHOWTITLE));   	/* Enter the directory */
					if(fresult != FR_OK) {
						break;
					}
					path[i] = 0;
				}
			} else {   /* It is a file. */
				 FH_printf(FAT_TAG, "\t[File] %s/%s\n", path, fno.fname);
			}
		}
		f_closedir(&dir);
		if(flags & SCAN_SHOWTITLE) {
			FH_printf(FAT_TAG, "\n");
		}
	} else {
		FH_ERR(FAT_TAG, "\topen DIR[%s] err[%d]\n", path, fresult);
	}
	
	return fresult;
}

// FRESULT FH_scan(StoreDisk_t* disk, char* scanPath)
// {
// 	return FH_scan_(disk, scanPath, 1);
// }

// can NOT remove the root(/) dir
FRESULT FH_remove_dir(StoreDisk_t* disk, char* path)
{
	DIR dir;
	FILINFO finfo;
	char full_path[MAX_PATH_LEN*2];
	
	if(disk == NULL || path == NULL || path[0] == '/') {
		FH_ERR(FAT_TAG, "\tparam err\n");
		return FR_INVALID_PARAMETER;
	}
	
	if(!FH_is_exist(disk, path)) {
		FH_ERR(FAT_TAG, "\t%s%s NOT exist\n", disk->name, path);
		return FR_NO_PATH;
	}
	
	if(strlen(disk->name) + strlen(path) >= MAX_PATH_LEN) {
		FH_ERR(FAT_TAG, "\tpath too long\n");
		return FR_INVALID_PARAMETER;
	}
	sprintf(full_path, "%s%s", disk->name, path);
	
	//FH_DBG("f_opendir(\"%s\")\n", full_path);
	fresult = f_opendir(&dir, full_path);
	if(fresult != FR_OK) {
		FH_ERR(FAT_TAG, "\tf_opendir() err[%d]\n", fresult);
		return fresult;
	}
	
	uint8_t len = strlen(full_path);
	while(1) {
		full_path[len] = '\0';
		fresult = f_readdir(&dir, &finfo);
		if(fresult != FR_OK) {
			FH_ERR(FAT_TAG, "\tf_readdir() err[%d]\n", fresult);
			break;
		}
		if(finfo.fname[0] == 0) {
			break;	// end of dir
		}
		//FH_DBG("\tpath: %s/%s\n", full_path, finfo.fname);
		if(strlen(full_path) + 1 + strlen(finfo.fname) >= MAX_PATH_LEN) {
			FH_ERR(FAT_TAG, "\tpath too long\n");
			fresult = FR_INVALID_PARAMETER;
			break;
		}
		sprintf(&full_path[len], "/%s", finfo.fname);
		if(finfo.fattrib & AM_DIR) {    /* It is a directory */
			fresult = FH_remove_dir(disk, &full_path[3]);
			if(fresult != FR_OK) {
				break;
			}
		} else {	// is a file
			fresult = f_unlink(full_path);
			if(fresult != FR_OK) {
				FH_ERR(FAT_TAG, "\tf_unlink(\"%s\") err[%d]\n", full_path, fresult);
				break;
			} else {
				FH_DBG(FAT_TAG, "\trm file: %s\n", full_path);
			}
		}
	}
	
	if(f_closedir(&dir)) {
		FH_ERR(FAT_TAG, "\tf_closedir() err\n");
	}
	
	if(fresult != FR_OK) {
		return fresult;
	}

	fresult = f_unlink(full_path);
	if(fresult != FR_OK) {
		FH_ERR(FAT_TAG, "\tf_unlink(\"%s\") err[%d]\n", full_path, fresult);
		return fresult;
	} else {
		FH_DBG(FAT_TAG, "\trm dir : %s\n", full_path);
	}
	
	return FR_OK;
}

/* Only supports removing files from home directory */
FRESULT FH_format(StoreDisk_t* disk)
{
	DIR dir;
	char fname[MAX_PATH_LEN];

	fresult = f_opendir(&dir, disk->name);     		/* Open the directory */
	if(fresult == FR_OK) {
		for(;;) {
			fresult = f_readdir(&dir, &fno);    	/* Read a directory item */
			if(fresult != FR_OK) {
				FH_ERR(FAT_TAG, "f_readdir(\"%s\") err[%d]\n", disk->name, fresult);
				break;
			}
			if(fno.fname[0] == 0) {
				break;		// end of dir
			}
			if(fno.fattrib & AM_DIR) {    /* It is a directory */
				if(!(strcmp ("SYSTEM~1", fno.fname))) {
					continue;
				}
				fresult = FH_remove_dir(disk, fno.fname);
				if(fresult != FR_OK) {
					FH_ERR(FAT_TAG, "FH_remove_dir(\"%s%s\") err[%d]\n", disk->name, fno.fname, fresult);
					break;
				}
			} else {   /* It is a file. */
				sprintf(fname, "%s%s", disk->name, fno.fname);
				fresult = f_unlink(fname);
				if(fresult != FR_OK) {
					FH_ERR(FAT_TAG, "f_unlink(\"%s\") err[%d]\n", fname, fresult);
					break;
				} else {
					FH_DBG(FAT_TAG, "rm file: %s\n", fname);
				}
			}
		}
		if(f_closedir(&dir) != FR_OK) {
			FH_ERR(FAT_TAG, "f_closedir() err\n");
		}
	} else {
		FH_ERR(FAT_TAG, "f_opendir() err[%d]\n", fresult);
	}
	return fresult;
}

uint8_t FH_is_exist(StoreDisk_t* disk, char* path)
{
	char full_path[MAX_PATH_LEN*2] = {0};
	
	if(strlen(disk->name) + strlen(path) >= MAX_PATH_LEN) {
		FH_ERR(FAT_TAG, "path too long\n");
		return FR_INVALID_PARAMETER;
	}
	sprintf(full_path, "%s%s", disk->name, path);
	
	fresult = f_stat(full_path, &fno);
	if (fresult != FR_OK)
	{
		//FH_DBG("\"%s\" NOT exists\n", full_path);
		return 0;
	}
	//FH_DBG("\"%s\" exist\n", full_path);
	return 1;
}

FRESULT FH_write(StoreDisk_t* disk, char *name, char *data, BYTE mode)
{
	UINT bw;
	char full_name[MAX_PATH_LEN*2] = {0};
	
	if(strlen(disk->name) + strlen(name) >= MAX_PATH_LEN) {
		FH_ERR(FAT_TAG, "path too long\n");
		return FR_INVALID_PARAMETER;
	}
	sprintf(full_name, "%s%s", disk->name, name);

	/* Create a file with read write access and open it */
	fresult = f_open(disk->file, full_name, FA_WRITE | mode);
	if(fresult != FR_OK) {
		FH_ERR(FAT_TAG, "open \"%s\" err[%d]\n", full_name, fresult);
		return fresult;
	}

	fresult = f_write(disk->file, data, strlen(data), &bw);
	if(fresult != FR_OK) {
		FH_ERR(FAT_TAG, "write \"%s\" err[%d]\n", full_name, fresult);
	}

	fresult = f_close(disk->file);
	if(fresult != FR_OK) {
		FH_ERR(FAT_TAG, "close \"%s\" err[%d]\n", full_name, fresult);
	}
	return fresult;
}

FRESULT FH_read(StoreDisk_t* disk, char* file_name, char* buf, UINT btr, UINT* br)
{
	char full_name[MAX_PATH_LEN*2] = {0};
	
	if(strlen(disk->name) + strlen(file_name) >= MAX_PATH_LEN) {
		FH_ERR(FAT_TAG, "path too long\n");
		return FR_INVALID_PARAMETER;
	}
	sprintf(full_name, "%s%s", disk->name, file_name);
	
	/**** check whether the file exists or not ****/
	fresult = f_stat(full_name, &fno);
	if(fresult != FR_OK) {
		FH_ERR(FAT_TAG, "file \"%s\" NOT exists\n", full_name);
	  return fresult;
	}

	fresult = f_open(disk->file, full_name, FA_READ);
	if(fresult != FR_OK) {
		FH_ERR(FAT_TAG, "open \"%s\" err[%d]\n", full_name, fresult);
		return fresult;
	}
	
	fresult = f_read(disk->file, buf, btr-1, br);
	if(fresult != FR_OK) {
		br = 0;
		buf[0] = '\0';
		FH_ERR(FAT_TAG, "read \"%s\" err[%d]\n", full_name, fresult);
	} else {
		buf[*br] = '\0';
		FH_DBG(FAT_TAG, "[%s]", buf);
		fresult = f_close(disk->file);
		if(fresult != FR_OK) {
			FH_ERR(FAT_TAG, "close \"%s\" err[%d]\n", full_name, fresult);
		}
	}
	return fresult;
}

FRESULT FH_read_bin(StoreDisk_t* disk, char* file_name, uint32_t offset, uint32_t numBytesToRead, uint8_t* dest)
{
	char full_name[MAX_PATH_LEN*2] = {0};
	if(strlen(disk->name) + strlen(file_name) >= MAX_PATH_LEN) {
		FH_ERR(FAT_TAG, "path too long\n");
		return FR_INVALID_PARAMETER;
	}
	sprintf(full_name, "%s%s", disk->name, file_name);
	
	fresult = f_stat(full_name, &fno);
	if (fresult != FR_OK) {
		FH_ERR(FAT_TAG, "\"%s\" does not exists\n", full_name);
	  return fresult;
	}
	
	fresult = f_open(disk->file, full_name, FA_READ);
	if(fresult != FR_OK) {
		FH_ERR(FAT_TAG, "opening file \"%s\" err[%d]\n", full_name, fresult);
		return fresult;
	}
	
	UINT numBytesRead = 0;
	f_lseek(disk->file, offset);
	f_read(disk->file, dest, numBytesToRead, &numBytesRead);
	if (fresult != FR_OK || numBytesRead != numBytesToRead) {
		memset(dest, 0, numBytesToRead);
		FH_ERR(FAT_TAG, "read bin file \"%s\" err[%d]\n", full_name, fresult);
		f_close(disk->file);
		return fresult;
	}
	
	fresult = f_close(disk->file);
	if (fresult != FR_OK) {
		FH_ERR(FAT_TAG, "close file[%s] err[%d]\n", full_name, fresult);
	}
	return fresult;
}

FRESULT FH_create_file(StoreDisk_t* disk, char* file_name)
{
	char full_name[MAX_PATH_LEN*2] = {0};
	if(strlen(disk->name) + strlen(file_name) >= MAX_PATH_LEN) {
		FH_ERR(FAT_TAG, "path too long\n");
		return FR_INVALID_PARAMETER;
	}
	sprintf(full_name, "%s%s", disk->name, file_name);
	
	fresult = f_stat(full_name, &fno);
	if(fresult == FR_OK) {
		FH_ERR(FAT_TAG, "\"%s\" already exists! use Update_File\n", full_name);
	  return fresult;
	} else {
		fresult = f_open(disk->file, full_name, FA_CREATE_ALWAYS|FA_READ|FA_WRITE);
		if(fresult != FR_OK) {
			FH_ERR(FAT_TAG, "create file \"%s\" err[%d]\n", full_name, fresult);
		  return fresult;
		} else {
			FH_DBG(FAT_TAG, "\"%s\" created ok. write by Write_File\n", full_name);
		}

		fresult = f_close(disk->file);
		if(fresult != FR_OK) {
			FH_ERR(FAT_TAG, "close file \"%s\" err[%d]\n", full_name, fresult);
		}
	}
	return fresult;
}

FRESULT FH_update_file(StoreDisk_t* disk, char* file_name, char *data)
{
	UINT bw;
	char full_name[MAX_PATH_LEN*2] = {0};
	
	if(strlen(disk->name) + strlen(file_name) >= MAX_PATH_LEN) {
		FH_ERR(FAT_TAG, "path too long\n");
		return FR_INVALID_PARAMETER;
	}
	sprintf(full_name, "%s%s", disk->name, file_name);
	
	/**** check whether the file exists or not ****/
	fresult = f_stat(full_name, &fno);
	if(fresult != FR_OK) {
		FH_ERR(FAT_TAG, "\"%s\" NOT exist\n", full_name);
	  return fresult;
	}
	
	/* Create a file with read write access and open it */
	fresult = f_open(disk->file, full_name, FA_OPEN_APPEND | FA_WRITE);
	if(fresult != FR_OK) {
		FH_ERR(FAT_TAG, "open \"%s\" err[%d]\n", full_name, fresult);
		return fresult;
	}

	/* Writing text */
	fresult = f_write(disk->file, data, strlen(data), &bw);
	if(fresult != FR_OK) {
		FH_ERR(FAT_TAG, "write \"%s\" err[%d]\n", full_name, fresult);
	}

	/* Close file */
	fresult = f_close(disk->file);
	if(fresult != FR_OK) {
		FH_ERR(FAT_TAG, "close \"%s\" err[%d]\n", full_name, fresult);
	}
	return fresult;
}

FRESULT FH_remove_file(StoreDisk_t* disk, char* file_name)
{
	char full_name[MAX_PATH_LEN*2] = {0};
	if(strlen(disk->name) + strlen(file_name) >= MAX_PATH_LEN) {
		FH_ERR(FAT_TAG, "path too long\n");
		return FR_INVALID_PARAMETER;
	}
	sprintf(full_name, "%s%s", disk->name, file_name);
	
	/**** check whether the file exists or not ****/
	fresult = f_stat(full_name, &fno);
	if(fresult != FR_OK){
		FH_ERR(FAT_TAG, "file \"%s\" not exist\n", full_name);
		return fresult;
	}

	fresult = f_unlink(full_name);
	if (fresult == FR_OK) {
		FH_DBG(FAT_TAG, "remove \"%s\" OK\n", full_name);
	} else {
		FH_ERR(FAT_TAG, "remove \"%s\" err[%d]\n", full_name, fresult);
	}
	return fresult;
}

FRESULT FH_get_file_info(StoreDisk_t* disk, char* file_name)
{
	char full_name[MAX_PATH_LEN*2] = {0};
	if(strlen(disk->name) + strlen(file_name) >= MAX_PATH_LEN) {
		FH_ERR(FAT_TAG, "path too long\n");
		return FR_INVALID_PARAMETER;
	}
	sprintf(full_name, "%s%s", disk->name, file_name);
	
	fresult = f_stat(full_name, &fno);
	if (fresult != FR_OK) {
		FH_ERR(FAT_TAG, "get file[%s] info err[%d]\n", full_name, fresult);
		return fresult;
	}
	
	FH_printf(FAT_TAG, "\n****** file: %s *******\n", full_name);
	FH_printf(FAT_TAG, "\tAttr: 0x%02x\n", fno.fattrib);
	FH_printf(FAT_TAG, "\tSize: %d Bytes\n", (int)fno.fsize);
	FH_printf(FAT_TAG, "\tDate: %04d-%02d-%02d %02d:%02d:%02d\n\n", 
											1980 + (fno.fdate >> 9), 
											(fno.fdate >> 5) & 0xF,
											fno.fdate & 0x1F,
											(fno.ftime >> 11) & 0x1F,
											(fno.ftime >> 5) & 0x3F,
											fno.ftime & 0x1F);
	return fresult;
}

FRESULT FH_create_dir(StoreDisk_t* disk, char* dir_name)
{
	char full_path[MAX_PATH_LEN*2] = {0};
	if(strlen(disk->name) + strlen(dir_name) >= MAX_PATH_LEN) {
		FH_ERR(FAT_TAG, "path too long\n");
		return FR_INVALID_PARAMETER;
	}
	sprintf(full_path, "%s%s", disk->name, dir_name);
	
	fresult = f_mkdir(full_path);
	if(fresult == FR_OK) {
		FH_DBG(FAT_TAG, "creat dir[%s]\n", full_path);
	} else {
		FH_ERR(FAT_TAG, "creat dir[%s] err[%d]\n", full_path, fresult);
	}
	return fresult;
}

void FH_get_space(StoreDisk_t* disk)
{
	DWORD fre_clust;
	uint32_t total, free_space;
	
    fresult = f_getfree(disk->name, &fre_clust, &disk->fatfs);
	if(fresult != FR_OK) {
		FH_ERR(FAT_TAG, "\tf_getfree() err[%d]\n", fresult);
		return;
	}
	FH_DBG(FAT_TAG, "\tfatent[%ld] csize[%d] frrClust[%ld]\n", disk->fatfs->n_fatent, disk->fatfs->csize, fre_clust);
    total = (uint32_t)((disk->fatfs->n_fatent - 2) * disk->fatfs->csize / 2);
	free_space = (uint32_t)(fre_clust * disk->fatfs->csize / 2);
	
	FH_printf(FAT_TAG, "\t[%s] space info(unit: KiB):\n", disk->name);
	FH_printf(FAT_TAG, "\t       Used         Free        Total  Percent\n");
	//		     16000000000  16000000000  16000000000 
	FH_printf(FAT_TAG, "\t%11u  %11u  %11u   %.2f\n", total - free_space, free_space, total, 100 - free_space * 100.0 / total);
}

#if FH_API_TEST
// see details of sDisk in header
void FH_API_test(StoreDisk_t* disk)
{
	#define FILE_SIZE	(100)	// How many bytes you want to read from file(MUST +1)
	char buf[FILE_SIZE + 1] = {0};
	UINT br = 0;

	/** @fixme: make filesystem for SPI Flash err, maybe the config for SPI Flash is something err */
	FH_mkfs(disk);
	
	FH_mount(disk);
	FH_format(disk);
	FH_create_dir(disk, "dir1");
	FH_create_file(disk, "dir1/file11.txt");
	FH_create_file(disk, "dir1/file12.txt");
	FH_create_dir(disk, "dir1/dir11");
	FH_create_file(disk, "dir1/dir11/file111.txt");
	FH_create_file(disk, "dir1/dir11/file112.txt");
	FH_create_dir(disk, "dir1/dir12");
	FH_create_file(disk, "dir1/dir12/file121.txt");
	FH_create_file(disk, "dir1/dir12/file122.txt");
	// FH_create_file(disk, "dir2/file21.txt");	// err, dir2 NOT exist
	// FH_create_dir(disk, "dir3/dir31");		// err, dir3 NOT exist
	
	FH_scan(disk, NULL, SCAN_SHOWTITLE | SCAN_RECURSIVE);
	FH_scan(disk, "dir1/dir12", SCAN_SHOWTITLE | SCAN_RECURSIVE);
	FH_remove_dir(disk, "dir1/dir12");
	FH_remove_file(disk, "dir1/file11.txt");
	FH_scan(disk, NULL, SCAN_SHOWTITLE | SCAN_RECURSIVE);
	
	FH_write(disk, "dir1/file12.txt", "FH_write() FA_OPEN_EXISTING test", FA_OPEN_EXISTING);
	FH_write(disk, "dir1/file12.txt", "FH_write() FA_CREATE_ALWAYS test", FA_CREATE_ALWAYS);
	FH_read(disk, "dir1/file12.txt", buf, FILE_SIZE, &br);
	printf("read file[%d]: [%s]\n", br, buf);

	FH_write(disk, "dir1/file12.txt", "FH_write() FA_OPEN_APPEND test", FA_OPEN_APPEND);
	FH_read(disk, "dir1/file12.txt", buf, FILE_SIZE, &br);
	printf("read file[%d]: [%s]\n", br, buf);
	
	FH_write(disk, "dir1/file12.txt", "FH_write() FA_CREATE_ALWAYS test", FA_CREATE_ALWAYS);
	FH_read(disk, "dir1/file12.txt", buf, FILE_SIZE, &br);
	printf("read file[%d]: [%s]\n", br, buf);
	
	FH_write(disk, "dir1/no_file.txt", "FH_write() FA_CREATE_ALWAYS test", FA_CREATE_ALWAYS);
	// FH_write(disk, "no_dir/no_file.txt", "FH_write() FA_CREATE_ALWAYS test", FA_CREATE_ALWAYS);		// ERR, no_dir is NOT exist
	
	FH_get_file_info(disk, "dir1/no_file.txt");
	
	HAL_Delay(500);
	
	/** @fixme: get SPI Flash space err */
	FH_get_space(disk);
	
	FH_unmount(disk);
	printf("\nFH API test finish\n");
}
#endif // FH_API_TEST
