#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CFG_UF2_FLASH_SIZE
#define CFG_UF2_FLASH_SIZE (2 * 1024 * 1024)
#endif

#define MAX_BLOCKS (CFG_UF2_FLASH_SIZE / 256 + 100)

typedef struct {
    uint32_t numBlocks;
    uint32_t numWritten;
    uint8_t writtenMask[MAX_BLOCKS / 8 + 1];
} WriteState;

int32_t uf2_write_block(uint32_t block_no, uint8_t* data, WriteState* state);

#ifdef __cplusplus
}
#endif