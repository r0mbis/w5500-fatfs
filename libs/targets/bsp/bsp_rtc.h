#pragma once

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

#define BOOT_RTC_SIGNATURE 0x4bd71066 
#define BOOT_RTC_UPDATE_WRELOAD 0x12d7347f 


void bsp_rtc_init(void);
uint32_t bsp_rtc_get_signature(void);
void bsp_rtc_set_signature(uint32_t sig);


#ifdef __cplusplus
}
#endif
