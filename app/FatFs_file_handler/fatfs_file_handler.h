/**
 * @brief   FATFS User API
 * 
 * @author  shadowthreed@gmail.com
 * @date    20240813
 *
 * @HOW
	0. redirection the printf();
	1. difine the sDisk(SDPath/SDFatFS/SDFile already define in fatfs.c)
		StoreDisk_t sDisk[_VOLUMES] = {
			// path       FATFS       FIL        opt     mCnt
			{SDPath, 	&SDFatFS, 	&SDFile, 	FM_FAT32, 0},		// SDcard disk
			{USERPath, 	&USERFatFS, &USERFile, 	FM_FAT,   0}		// SpiFlash disk
		};
	2. refer the FH_API_test();
 */

#ifndef __FATFS_FILE_HANDLER_H__
#define __FATFS_FILE_HANDLER_H__

#include "ff.h"
#include "string.h"

#define SD_LOG_EN		1
#define MAX_PATH_LEN	50		// full path length(include volume name, eg: "0:/DirName/FileName.xxx" length is 23)
#define FH_API_TEST		1

#define SCAN_SHOWTITLE	0x01
#define SCAN_RECURSIVE	0x02
// define yourself's LOG API
#if SD_LOG_EN
#include "sys_log.h"
//#define FH_DBG
//#define FH_ERR
#endif
//#define FH_printf

typedef struct {
	const char* const name;
	FATFS* fatfs;
	FIL* file;
	const MKFS_PARM* opt;
	uint8_t mount_cnt;
} StoreDisk_t;

void FH_mkfs(StoreDisk_t* disk);
void FH_mount(StoreDisk_t* disk);
void FH_unmount(StoreDisk_t* disk);

/**
 * @brief delete all the file and dir in the disk.
 */
FRESULT FH_format(StoreDisk_t* disk);

/**
 * @brief	judge if a file or dir is exist.
 * @param	path eg: "dirName/fileNeme.txt", "dirName"
 * @ret		0 not exist
 *			1 exist
 */
uint8_t FH_is_exist(StoreDisk_t* disk, char* path);

/**
 * @note	MUST create the DIR in the exist DIR!
 */
FRESULT FH_create_dir(StoreDisk_t* disk, char* dir_name);

/**
 * @brief	delete the dir and all the files in this dir;
 * @note	path can't be root(/) dir; path can't be NULL;
 *			if want to remove "0:/dir1/dir2", path should be "dir1/dir2", it will remove dir2 only, dir1 will keep.
 */
FRESULT FH_remove_dir(StoreDisk_t* disk, char* path);

/**
 * @brief	scan the specified dir; for scan "0:/dir/subDir", the scanPath should be "dir/subDir";
 * @note	if need scan the whole disk, scanPath should be NULL;
 */
FRESULT FH_scan(StoreDisk_t* disk, char* scanPath, uint8_t flags);

/**
 * @note	MUST create the file in the exist DIR!
 */
FRESULT FH_create_file(StoreDisk_t* disk, char* file_name);
FRESULT FH_remove_file(StoreDisk_t* disk, char* file_name);

/**
 * @param	
	mode
	 FA_OPEN_EXISTING: Opens the file. The function fails if the file is not existing. (Default)
	 FA_OPEN_ALWAYS: Opens the file if it is existing. If not, a new file is created.
					To append data to the file, use f_lseek function after file open in this method.
	 FA_CREATE_NEW: Creates a new file. The function fails with FR_EXIST if the file is existing.
	 FA_CREATE_ALWAYS: Creates a new file. If the file is existing, it is truncated and overwritten.
	 FA_OPEN_APPEND: apend file
 *
 * @note	the file's parent DIR MUST exist!
 */
FRESULT FH_write(StoreDisk_t* disk, char* file_name, char *data, BYTE mode);
FRESULT FH_read(StoreDisk_t* disk, char* file_name, char* buf, UINT btr, UINT* br);
FRESULT FH_read_bin(StoreDisk_t* disk, char* file_name, uint32_t offset, uint32_t numBytesToRead, uint8_t* dest);

FRESULT FH_get_file_info(StoreDisk_t* disk, char* file_name);
void FH_get_space(StoreDisk_t* disk);

#if FH_API_TEST
void FH_API_test(StoreDisk_t* disk);
#endif // FH_API_TEST

#endif /* __FATFS_FILE_HANDLER_H__ */
