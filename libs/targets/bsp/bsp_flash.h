#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t sector_number;
    uint32_t size;
    uint32_t bank;
}flash_sectors_t;

bool bsp_flash_is_invalid(void);
void bsp_flash_set_invalid(void);

bool bsp_flash_check_crc(uint32_t addr, uint32_t descr_addr);

void bsp_flash_write(uint32_t dst, const uint8_t* src, uint32_t len );
void bsp_flash_read(uint32_t addr, void* buffer, uint32_t len);
void bsp_flash_flush(void);


#ifdef __cplusplus
}
#endif