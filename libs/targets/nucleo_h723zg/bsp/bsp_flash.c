#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "bsp_flash.h"
#include "bsp_boot.h"
#include "crc.h"

#include "uf2_config.h"
#include "stm32h7xx_hal_flash.h"
#include "stm32h7xx_hal_flash_ex.h"

#include "sys_log.h"
#include "sys_descriptor.h"
#include "sys_compiler.h"


#define TAG "FLASH"

static uint8_t __corrupted = 0;


/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0_BANK1 ((uint32_t)0x08000000) /* Base @ of Sector 0, Bank1, 128 Kbyte */
#define ADDR_FLASH_SECTOR_1_BANK1 ((uint32_t)0x08020000) /* Base @ of Sector 1, Bank1, 128 Kbyte */
#define ADDR_FLASH_SECTOR_2_BANK1 ((uint32_t)0x08040000) /* Base @ of Sector 2, Bank1, 128 Kbyte */
#define ADDR_FLASH_SECTOR_3_BANK1 ((uint32_t)0x08060000) /* Base @ of Sector 3, Bank1, 128 Kbyte */
#define ADDR_FLASH_SECTOR_4_BANK1 ((uint32_t)0x08080000) /* Base @ of Sector 4, Bank1, 128 Kbyte */
#define ADDR_FLASH_SECTOR_5_BANK1 ((uint32_t)0x080A0000) /* Base @ of Sector 5, Bank1, 128 Kbyte */
#define ADDR_FLASH_SECTOR_6_BANK1 ((uint32_t)0x080C0000) /* Base @ of Sector 6, Bank1, 128 Kbyte */
#define ADDR_FLASH_SECTOR_7_BANK1 ((uint32_t)0x080E0000) /* Base @ of Sector 7, Bank1, 128 Kbyte */

#define FLASH_BANK_1             0x01U                         /*!< Bank 1   */

static uint8_t erasedSectors[CGF_UF2_BOARD_FLASH_SECTORS];


/* flash parameters that we should not really know */
static flash_sectors_t flash_sectors[] = {
    {0x00, 128 * 1024, FLASH_BANK_1},
    {0x01, 128 * 1024, FLASH_BANK_1},
    {0x02, 128 * 1024, FLASH_BANK_1},
    {0x03, 128 * 1024, FLASH_BANK_1},
    {0x04, 128 * 1024, FLASH_BANK_1},
    {0x05, 128 * 1024, FLASH_BANK_1},
    {0x06, 128 * 1024, FLASH_BANK_1},
    {0x07, 128 * 1024, FLASH_BANK_1},
};



bool bsp_flash_is_invalid(){
    return __corrupted;
}

void bsp_flash_set_invalid(){
    __corrupted = 1;
}

bool bsp_flash_check_crc(uint32_t addr, uint32_t descr_addr) {
    uint32_t calculated_crc = 0;
    bool res = false;
    volatile sys_descriptor_t* descriptor = (sys_descriptor_t*)descr_addr;

    if(descriptor->magic_word != APP_DESCRIPTOR_MAGIC_WORD) {
        log_i(TAG, "No descriptor found");
        return false;
    }

    __HAL_RCC_CRC_FORCE_RESET();
    __HAL_RCC_CRC_RELEASE_RESET();

    calculated_crc = HAL_CRC_Calculate(&hcrc, (uint32_t*)addr, descriptor->size / 4);

    __HAL_RCC_CRC_FORCE_RESET();
    __HAL_RCC_CRC_RELEASE_RESET();

    if(descriptor->crc32 != calculated_crc) {
        log_e(TAG, "FW CRC Err! FLSH: %lx DSCR: %lx", calculated_crc, descriptor->crc32);
        res = false;
    } else {
        log_i(TAG, "FW CRC Ok! FLSH: %lx DSCR: %lx", calculated_crc, descriptor->crc32);
        res = true;
    }

    return res;
}


static uint32_t flash_func_sector_size(unsigned sector) {
    if(sector < CGF_UF2_BOARD_FLASH_SECTORS) {
        return flash_sectors[sector].size;
    }

    return 0;
}


static bool is_blank(uint32_t addr, uint32_t size) {
    for(unsigned i = 0; i < size; i += sizeof(uint32_t)) {
        if(*(uint32_t*)(addr + i) != 0xffffffff) {
            // log_i("erase", "non blank: %p i=%d/%d", addr, i, size);
            return false;
        }
    }
    return true;
}

static void flash_unlock() {
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGSERR | FLASH_FLAG_WRPERR);
    HAL_FLASH_Unlock();
}

/*
 * Write flash, erase sector if necessary and not already erased.
 */
NO_OPT void bsp_flash_write(uint32_t dst, const uint8_t* src, uint32_t len) {

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

    if(!erasedSectors[sector]) {

        if(!is_blank(dst, size)) {
            FLASH_EraseInitTypeDef eraseInit;
            eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
            eraseInit.Banks = flash_sectors[sector].bank;
            eraseInit.Sector = flash_sectors[sector].sector_number;
            eraseInit.NbSectors = 1;
            eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

            uint32_t sectorError = 0;
            
            HAL_FLASHEx_Erase(&eraseInit, &sectorError);

            if(!is_blank(dst, size) | (sectorError != 0xffffffff)) {
                log_e(TAG, "Failed to erase %ld - %d", eraseInit.Sector, sectorError);
            }
        }


        // FLASH_Erase_Sector(sector,FLASH_BANK_1, FLASH_VOLTAGE_RANGE_3);
        // FLASH_WaitForLastOperation(HAL_MAX_DELAY, FLASH_BANK_1);

        // if(!is_blank(dst, flash_sectors[sector].size ) ){
        //     log_e(TAG, "Failed to erase %ld", sector);//, sectorError);
        // }

        erasedSectors[sector] = 1; // don't erase anymore - we will continue writing here!
    }

    // invalidate flash buffer after it may have been erased
    // SCB_InvalidateDCache_by_Addr(&dst, len);
    // check if flash is really empty, otherwise ECC errors might be created
    if(!is_blank(dst, len)) {
        log_e(TAG, "Flash to write is not empty! Reset");
        bsp_boot_send_reset();
    }

    for(uint32_t i = 0; i < len; i += 4) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, dst + i, (uint32_t)src + i);
    }

    // TODO: implement error checking
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

    FLASH_EraseInitTypeDef eraseInit;
    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    // eraseInit.Banks = FLASH_BANK_2;
    eraseInit.Sector = 0x07; //ADDR_FLASH_SECTOR_7_BANK2
    eraseInit.NbSectors = 1;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    uint32_t sectorError = 0;
    HAL_FLASHEx_Erase(&eraseInit, &sectorError);

    if(!is_blank(BOARD_FLASH_BACKUP_BOOT_START, 128 * 1024) | (sectorError != 0xffffffff)) {
        log_e(TAG, "Failed to erase backup sector");
        res = false;
    } else {
        log_i(TAG, "Erased backup sector..");
        res = true;
    }
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
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    uint32_t sectorError = 0;
    HAL_FLASHEx_Erase(&eraseInit, &sectorError);

    if(!is_blank(BOARD_FLASH_BOOT_START, 128 * 1024) | (sectorError != 0xffffffff)) {
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

    for(uint32_t i = 0; i < BOOTLOADER_SIZE - 3; i += 32) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD,
                          BOARD_FLASH_BACKUP_BOOT_START + i,
                          ((uint32_t)(BOARD_FLASH_BOOT_START + i)));
    }
    HAL_FLASH_Lock();

    log_i(TAG, "Backup sector write: DONE");
}

void write_from_backup() {
    log_i(TAG, "Boot sector write start...");

    flash_unlock();

    for(uint32_t i = 0; i < BOOTLOADER_SIZE - 3; i += 32) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD,
                          BOARD_FLASH_BOOT_START + i,
                          ((uint32_t)(BOARD_FLASH_BOOT_START + i)));
    }
    HAL_FLASH_Lock();

    log_i(TAG, "Boot sector write: DONE");
}

void backup_bootloader() {

    do {

        // check CRC in BACKUP SECTOR
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



