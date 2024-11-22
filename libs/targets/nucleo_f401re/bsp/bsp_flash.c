#include "bsp_flash.h"


#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "bsp_boot.h"
#include "crc.h"

#include "uf2_config.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_flash_ex.h"

#include "sys_log.h"
#include "sys_descriptor.h"
#include "sys_compiler.h"

#define TAG "FLASH"
// static uint8_t __corrupted = 0;

static uint8_t erasedSectors[CGF_UF2_BOARD_FLASH_SECTORS];

// /* Base address of the Flash sectors */
// static flash_sectors_t flash_sectors[] = {
//     {0U, 16 * 1024, FLASH_BANK_1},
// 	{1U, 16 * 1024, FLASH_BANK_1},
// 	{2U, 16 * 1024, FLASH_BANK_1},
// 	{3U, 16 * 1024, FLASH_BANK_1},
// 	// Application (BOARD_FLASH_APP_START)
// 	{4U, 64 * 1024, FLASH_BANK_1},
// 	{5U, 128 * 1024, FLASH_BANK_1},
// 	{6U, 128 * 1024, FLASH_BANK_1},
// 	{7U, 128 * 1024, FLASH_BANK_1},
// 	// flash sectors only in 1 MB devices
// 	{8U, 128 * 1024, FLASH_BANK_1},
// 	{9U, 128 * 1024, FLASH_BANK_1},
// 	{10U, 128 * 1024, FLASH_BANK_1},
// 	{11U, 128 * 1024, FLASH_BANK_1},
// 	// flash sectors only in 2 MB devices
// 	{0U, 16 * 1024, FLASH_BANK_2},
// 	{1U, 16 * 1024, FLASH_BANK_2},
// 	{2U, 16 * 1024, FLASH_BANK_2},
// 	{3U, 16 * 1024, FLASH_BANK_2},
// 	{4U, 64 * 1024, FLASH_BANK_2},
// 	{5U, 128 * 1024, FLASH_BANK_2},
// 	{6U, 128 * 1024, FLASH_BANK_2},
// 	{7U, 128 * 1024, FLASH_BANK_2},
// 	{8U, 128 * 1024, FLASH_BANK_2},
// 	{9U, 128 * 1024, FLASH_BANK_2},
// 	{10U, 128 * 1024, FLASH_BANK_2},
// 	{11U, 128 * 1024, FLASH_BANK_2},

// };

bool bsp_flash_is_invalid(){
    // return __corrupted;
    return true;
}

void bsp_flash_set_invalid(){
    // __corrupted = 1;
}

bool bsp_flash_check_crc(uint32_t addr, uint32_t descr_addr) {
    // uint32_t calculated_crc = 0;
    // bool res = false;
    // volatile sys_descriptor_t* descriptor = (sys_descriptor_t*)descr_addr;

    // if(descriptor->magic_word != APP_DESCRIPTOR_MAGIC_WORD) {
    //     log_i(TAG, "No descriptor found");
    //     return false;
    // }

    // __HAL_RCC_CRC_FORCE_RESET();
    // __HAL_RCC_CRC_RELEASE_RESET();

    // calculated_crc = HAL_CRC_Calculate(&hcrc, (uint32_t*)addr, descriptor->size / 4);

    // __HAL_RCC_CRC_FORCE_RESET();
    // __HAL_RCC_CRC_RELEASE_RESET();

    // if(descriptor->crc32 != calculated_crc) {
    //     log_e(TAG, "FW CRC Err! FLSH: %lx DSCR: %lx", calculated_crc, descriptor->crc32);
    //     res = false;
    // } else {
    //     log_i(TAG, "FW CRC Ok! FLSH: %lx DSCR: %lx", calculated_crc, descriptor->crc32);
    //     res = true;
    // }

    // return res;
    return true;
}



uint32_t flash_func_sector_size(unsigned sector) {
    // if(sector < CGF_UF2_BOARD_FLASH_SECTORS) {
        // return flash_sectors[sector].size;
    // }

    return 0;
}


static bool is_blank(uint32_t addr, uint32_t size) {
    for(unsigned i = 0; i < size; i += sizeof(uint32_t)) {
        if(*(uint32_t*)(addr + i) != 0xffffffff) {
            log_e("erase", "error: non blank: %p i=%d/%d", addr, i, size);
            return false;
        }
    }
    return true;
}

static void flash_unlock() {
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGSERR | FLASH_FLAG_WRPERR);

}


/*
 * Write flash, erase sector if necessary and not already erased.
 */
NO_OPT void  bsp_flash_write(uint32_t dst, const uint8_t* src, uint32_t len ) {

    uint32_t addr = 0x08000000;
    uint32_t sector = 0;
    uint32_t size = 0;

    for(unsigned i = 0; i < CGF_UF2_BOARD_FLASH_SECTORS; i++) {
        size = flash_func_sector_size(i);
        if(addr + size > dst) {
            sector = i;
            break;
        }
        addr += size;
    }
    
    if(sector >= CGF_UF2_BOARD_FLASH_SECTORS) { // sector == 0 allowed to update bootloader
        log_e(TAG, "Invalid sector - %ld, max - %d", sector, CGF_UF2_BOARD_FLASH_SECTORS);
        return; 
    }

    flash_unlock();

    if(!erasedSectors[sector]){ 
            FLASH_Erase_Sector(sector, FLASH_VOLTAGE_RANGE_3);
            FLASH_WaitForLastOperation(HAL_MAX_DELAY);

            // if(!is_blank(dst, flash_sectors[sector].size / 4) ){
            //     log_e(TAG, "Failed to erase %ld", sector);//, sectorError);
            // }

        erasedSectors[sector] = 1; // don't erase anymore - we will continue writing here!
    }

    // invalidate flash buffer after it may have been erased
    // check if flash is really empty, otherwise ECC errors might be created
    if(!is_blank(dst, len)) {
        log_e(TAG, "Flash to write is not empty! Reset");
        bsp_boot_send_reset();
    }


    for (uint32_t i = 0; (i < len / 4) && (dst <= (BOARD_FLASH_END - 4)); i++)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, dst,  *((uint32_t*) src + i)) == HAL_OK) {
            /* Check the written value */
            if (*(uint32_t*)dst != *((uint32_t*) src + i)) {
                log_e(TAG, "Write flash check fail");
            }
            dst += 4;
        } else {
            log_e(TAG, "Write flash fail");
        }
    }


    HAL_FLASH_Lock();
}
void bsp_flash_read(uint32_t addr, void* buffer, uint32_t len) {
    memcpy(buffer, (uint32_t*)addr, len);
}

void bsp_flash_flush(void) {
    memset(&erasedSectors, 0, CGF_UF2_BOARD_FLASH_SECTORS * sizeof(uint8_t));
}

bool erase_backup_sector() {
    bool res = false;
    flash_unlock();

    // FLASH_EraseInitTypeDef eraseInit;
    // eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    // eraseInit.Banks = FLASH_BANK_2;
    // eraseInit.Sector = 0x00; //ADDR_FLASH_SECTOR_7_BANK2
    // eraseInit.NbSectors = 128;
    // eraseInit.VoltageRange = 0x00000000U; //FLASH_VOLTAGE_RANGE_1;

    // uint32_t sectorError = 0;
    // HAL_FLASHEx_Erase(&eraseInit, &sectorError);

    // if(!is_blank(BOARD_FLASH_BACKUP_BOOT_START, 8 * 1024) | (sectorError != 0xffffffff)) {
    //     log_e(TAG, "Failed to erase backup sector");
    //     res = false;
    // } else {
    //     log_i(TAG, "Erased backup sector..");
    //     res = true;
    // }
    HAL_FLASH_Lock();

    return res;
}

bool erase_boot_sector() {
    bool res = false;

    flash_unlock();

    FLASH_EraseInitTypeDef eraseInit;
    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.Banks = FLASH_BANK_1;
    eraseInit.Sector = 0x00; //ADDR_FLASH_SECTOR_0_BANK1
    eraseInit.NbSectors = 1;
    eraseInit.VoltageRange = 0x00000000U;

    uint32_t sectorError = 0;
    HAL_FLASHEx_Erase(&eraseInit, &sectorError);

    if(!is_blank(BOARD_FLASH_BOOT_START, 8 * 1024) | (sectorError != 0xffffffff)) {
        log_e(TAG, "Failed to erase boot sector");
        res = false;
    } else {
        log_i(TAG, "Erased boot sector..");
        res = true;
    }
    HAL_FLASH_Lock();

    return res;
}

void write_to_backup() {
    log_i(TAG, "Backup sector write start...");

    flash_unlock();

    for(uint32_t i = 0; i < BOOTLOADER_SIZE ; i+= 16) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                          BOARD_FLASH_BACKUP_BOOT_START + i,
                          ((uint32_t)(BOARD_FLASH_BOOT_START + i)));
    }
    HAL_FLASH_Lock();

    log_i(TAG, "Backup sector write: DONE");
}

void write_from_backup() {
    log_i(TAG, "Boot sector write start...");

    flash_unlock();

    for(uint32_t i = 0; i < BOOTLOADER_SIZE ; i += 16) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                          BOARD_FLASH_BOOT_START + i,
                          ((uint32_t)(BOARD_FLASH_BOOT_START + i)));
    }
    HAL_FLASH_Lock();

    log_i(TAG, "Boot sector write: DONE");
}

void backup_bootloader() {

    do {

        // // check CRC in BACKUP SECTOR
        // if(compare_backup_crc()) {
        //     log_i(TAG, "Backup sector OK");
        //     break;
        // } else {
        //     log_w(TAG, "Backup CRC mismatch, updating..");
        // }

        // write bootloader to BACKUP SECTOR
        // erase_backup_sector();
        // write_to_backup();

        // check CRC
        // compare_backup_crc();

    } while(false);
}

bool restore_backup() {
    bool res = true;
    log_d(TAG, "RESTORING BACKUP..");
    do {

        if(!bsp_flash_check_crc(BOARD_FLASH_BACKUP_BOOT_START, BOARD_FLASH_BACKUP_BOOT_DESCRIPTOR)) {
            log_e(TAG, "YOU LOOSE");
            res = false;
            break;
        }

        erase_boot_sector();
        write_from_backup();

        // if(!compare_backup_crc()) {
        //     log_e(TAG, "CRC CHECK AFTER RESTORE FAILED");
        //     res = false;
        // }

    } while(false);

    if(res) log_d(TAG, "BACKUP RESTORED");

    return res;
}



