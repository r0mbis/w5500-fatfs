#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void bsp_boot_init_nvic(uint32_t addr );
void bsp_boot_jump_to_addr(uint32_t app_addr);
bool bsp_boot_nvic_valid(uint32_t app_addr);
void bsp_boot_send_reset();

#ifdef __cplusplus
}
#endif

