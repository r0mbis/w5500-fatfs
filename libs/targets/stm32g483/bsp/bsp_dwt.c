#include "bsp_dwt.h"
#include "stm32g4xx.h"

#define MCU_RATE (SystemCoreClock / 1000000)

void bsp_dwt_init() {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0U;
}

void bsp_dwt_delay_us(uint32_t microseconds) {
    uint32_t start = DWT->CYCCNT;
    uint32_t time_ticks = MCU_RATE * microseconds;
    while((DWT->CYCCNT - start) < time_ticks) {
    };
}

uint32_t bsp_dwt_instructions_per_microsecond() {
    return MCU_RATE;
}

bsp_dwt_t bsp_dwt_timer_get(uint32_t timeout_us) {
    bsp_dwt_t dwt = {0};
    dwt.start = DWT->CYCCNT;
    dwt.value = MCU_RATE * timeout_us;
    return dwt;
}

bool bsp_dwt_timer_is_expired(bsp_dwt_t dwt) {
    return !((DWT->CYCCNT - dwt.start) < dwt.value);
}

void bsp_dwt_timer_wait(bsp_dwt_t dwt) {
    while(!bsp_dwt_timer_is_expired(dwt));
}