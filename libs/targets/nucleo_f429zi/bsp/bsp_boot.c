#include "bsp_boot.h"
#include "sys_compiler.h"
#include "sys_descriptor.h"
#include "sys_log.h"

#include "stm32f4xx_hal.h"
#include <string.h>
#include "uf2_config.h"

#define TAG "startup"

extern uint32_t g_pfnVectors[];

NO_OPT void bsp_boot_init_nvic(uint32_t addr) {
    __disable_irq();

    if(!addr) {
        uint32_t vectorTable_RAM[256] __attribute__((aligned(0x200ul)));
        memcpy(vectorTable_RAM, g_pfnVectors, 256 * sizeof(uint32_t));
        SCB->VTOR = (uint32_t)&g_pfnVectors;
    } else {
        SCB->VTOR = addr;
    }

    __DSB();
    __ISB();
    __enable_irq();
}

NO_OPT void bsp_boot_jump_to_addr(uint32_t app_addr) {

    __disable_irq();
    for(int i = 0; i < 8; i++) {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }
    __set_CONTROL(0);

    __set_MSP(*(__IO uint32_t*)app_addr);

    uint32_t JumpAddress = *((volatile uint32_t*)(app_addr + 4));

    __ISB();
    __DSB();

    SysTick->CTRL &= ~(SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk);

    void (*reset_handler)(void) = (void*)JumpAddress;

    while(1) reset_handler();
}

bool bsp_boot_nvic_valid(uint32_t app_addr) {
    bool res = true;
    volatile uint32_t const* app_vector = (volatile uint32_t const*)app_addr;
    
    do {
        /* First word is the stack pointer (should be in D1_DTCMRAM_BASE region) */
        if(app_vector[0] < 0x20000000UL ||
           app_vector[0] > 0x20000000UL + (2 * 1024 * (1024 - 128))) {
            log_e(TAG, "FW: Invalid stack pointer");
            res = false;
            break;
        }

        /* Second word is App entry point (reset vector) */
        if(app_vector[1] < BOARD_FLASH_APP_START || app_vector[1] > BOARD_FLASH_END) {
            log_e(TAG, "FW: Invalid entry point");
            res = false;
            break;
        }


    } while(false);

    return res;
}

void bsp_boot_send_reset(void){
    NVIC_SystemReset();
}