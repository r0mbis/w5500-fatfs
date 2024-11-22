/*
 * The MIT License (MIT)
 *
 * Copyright (c) Microsoft Corporation
 * Copyright (c) Ha Thach for Adafruit Industries
 * Copyright (c) Henry Gabryjelski
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "uf2.h"
#include "uf2_block.h"
#include "uf2_config.h"

#include "bsp_flash.h"
#include "bsp_boot.h"

#include "sys_log.h"
#include "sys_descriptor.h"

#include "aes.h"
#include "keys.h"


#define TAG "UF2"
#define UF2_SECTOR_SIZE (512)

static volatile bool skip_rest = false;

static inline bool is_uf2_block(const void* data) {
    const UF2_Block* bl = (const UF2_Block*)data;
    return bl->magicStart0 == UF2_MAGIC_START0 && bl->magicStart1 == UF2_MAGIC_START1 &&
           bl->magicEnd == UF2_MAGIC_END;
}

static void reset_uf(WriteState* state) {
    bsp_flash_flush();
    *state = (WriteState){0};
}

/**
 * Write an uf2 block wrapped by 512 sector.
 * @return number of bytes processed, only 3 following values
 *  -1 : if not an uf2 block
 * 512 : write is successful (UF2_SECTOR_SIZE == 512)
 *   0 : is busy with flashing, tinyusb stack will call write_block again with the same parameters later on
 */

int32_t uf2_write_block(uint32_t block_no, uint8_t* data, WriteState* state) {
    (void)block_no;

    struct AES_ctx ctx;

    UF2_Block* bl = (void*)data;

    if(!is_uf2_block(bl) || skip_rest) {
        return -1;
    } else if(bl->flags & Uf2_FLAG_CRC) {

        uint32_t new_crc32 = bl->crc[0];
        uint32_t file_size = bl->crc[1];
        log_e(TAG, "Update binary size: %ld bytes", file_size);

        AES_init_ctx_iv(&ctx, local_key, local_iv);
        AES_CBC_decrypt_buffer(&ctx, (uint8_t*)&bl->crc[2], 4);

        if(bl->familyID == FIRMWARE_UF2_FAMILY_ID) {

            volatile const sys_descriptor_t* app_descriptor =
                (volatile const sys_descriptor_t*)BOARD_FLASH_APP_DESCRIPTOR;
            log_i(TAG, "Firmware: %x >> %x", app_descriptor->crc32, new_crc32);

            if(bl->crc[2] != APP_DESCRIPTOR_MAGIC_WORD) {
                log_e(TAG, "Encryption error");
                state->numWritten = state->numBlocks;
                skip_rest = true;

            } else if(app_descriptor->crc32 == new_crc32) {
                log_e(TAG, "CRC MATCH");
                state->numWritten = state->numBlocks;
                skip_rest =  
                    !bsp_flash_is_invalid(); // not skipping if flashing same fw over corrupted one

            } else {
                skip_rest = false;
            }

        } else if(bl->familyID == BOOTLOADER_UF2_FAMILY_ID) {

            volatile const sys_descriptor_t* app_descriptor =
                (volatile const sys_descriptor_t*)BOARD_FLASH_BOOT_DESCRIPTOR;

            log_i(TAG, "Bootloader: %x >> %x", app_descriptor->crc32, new_crc32);

            if(bl->crc[2] != APP_DESCRIPTOR_MAGIC_WORD) {

                log_e(TAG, "Encryption error");

                state->numWritten = state->numBlocks;
                skip_rest = true;

            } else if(app_descriptor->crc32 == new_crc32) {

                log_e(TAG, "CRC MATCH");

                state->numWritten = state->numBlocks;
                skip_rest = true;

            } else {
                skip_rest = false;
            }
        }

    } else if(bl->familyID == FIRMWARE_UF2_FAMILY_ID) {

        if(bl->blockNo == 1) {
            log_dn(TAG, "Firmware update ");
        } else if((bl->blockNo & 15) == 0) {
            sys_log_print_str(".");
        }

        if(bl->flags & Uf2_FLAG_PAYLOAD_ENCRYPTED) {
            AES_init_ctx_iv(&ctx, local_key, local_iv);
            AES_CBC_decrypt_buffer(&ctx, bl->data, bl->payloadSize);
        }

        bsp_flash_write(bl->targetAddr, bl->data, bl->payloadSize);

    } else if(bl->familyID == BOOTLOADER_UF2_FAMILY_ID) {

        // set RTC SIG?
        if(bl->blockNo == 1) {
            log_dn(TAG, "Bootloader update ");
        } else if((bl->blockNo & 15) == 0) {
            sys_log_print_str(".");
        }

        if(bl->flags & Uf2_FLAG_PAYLOAD_ENCRYPTED) {
            AES_init_ctx_iv(&ctx, local_key, local_iv);
            AES_CBC_decrypt_buffer(&ctx, bl->data, bl->payloadSize);
        }

        bsp_flash_write(bl->targetAddr, bl->data, bl->payloadSize);

    } else {
        return -1;
    }

    // Update written blocks if crc from first block mismatched
    if(bl->numBlocks) {

        // Update state num blocks if needed
        if(state->numBlocks != bl->numBlocks) {
            if(bl->numBlocks >= MAX_BLOCKS || state->numBlocks) {
                state->numBlocks = 0xffffffff;
            } else {
                state->numBlocks = bl->numBlocks;
            }
        }

        if(bl->blockNo < MAX_BLOCKS) {
            uint8_t const mask = 1 << (bl->blockNo % 8);
            uint32_t const pos = bl->blockNo / 8;

            // only increase written number with new write (possibly prevent overwriting from OS)
            if(!(state->writtenMask[pos] & mask)) {
                state->writtenMask[pos] |= mask;
                state->numWritten++;
            }

            // flush last blocks
            // TODO numWritten can be smaller than numBlocks if return early
            if(state->numWritten >= state->numBlocks) {
                sys_log_print_str("\r\n");
                if(bl->familyID == BOOTLOADER_UF2_FAMILY_ID) {

                    if(bsp_flash_check_crc(BOARD_FLASH_BOOT_START, BOARD_FLASH_BOOT_DESCRIPTOR)) {
                        log_i(TAG, "Bootloader update complete! Restarting..");
                    } else {
                        log_i(TAG, "Bootloader update failed!");
                    }

                } else {

                    if(bsp_flash_check_crc(BOARD_FLASH_APP_START, BOARD_FLASH_APP_DESCRIPTOR)) {
                        log_i(TAG, "Firmware update complete!");
                    } else {
                        log_i(TAG, "Update failed! Please retry.");
                    }
                }

                reset_uf(state);
            }
        }
    }

    return UF2_SECTOR_SIZE;
}