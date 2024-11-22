#include "bsp_mcu.h"
#include "sys_log.h"
#include "sys_utils.h"
#include "stm32h7xx_hal.h"

#define TAG "MCU INFO"

// address of MCU IDCODE
#define DBGMCU_IDCODE 0x5C001000 //0xE00E1000
#define STM32_UNKNOWN 0
#define STM32H7x3 0x450

#define REVID_MASK 0xFFFF0000
#define DEVID_MASK 0xFFF

/* magic numbers from reference manual */
typedef enum mcu_rev_e {
    MCU_REV_STM32H7_REV_Z = 0x1001,
    MCU_REV_STM32H7_REV_Y = 0x1003,
    MCU_REV_STM32H7_REV_V = 0x2003,
} mcu_rev_e;

typedef struct mcu_des_t {
    uint16_t mcuid;
    const char* desc;
    char rev;
} mcu_des_t;

typedef struct mcu_rev_t {
    mcu_rev_e revid;
    char rev;
} mcu_rev_t;

// The default CPU ID  of STM32_UNKNOWN is 0 and is in offset 0
// Before a rev is known it is set to ?
// There for new silicon will result in STM32F4..,?
mcu_des_t mcu_descriptions[] = {
    {STM32_UNKNOWN, "STM32H???", '?'},
    {STM32H7x3, "STM32H7x3", '?'},
};

const mcu_rev_t silicon_revs[] = {
    {MCU_REV_STM32H7_REV_Z, 'Z'},
    {MCU_REV_STM32H7_REV_Y, 'Y'},
    {MCU_REV_STM32H7_REV_V, 'V'},
};

static int bsp_mcu_get_mcu_descr(int max, uint8_t* revstr) {
    uint32_t idcode = (*(uint32_t*)DBGMCU_IDCODE);
    int32_t mcuid = idcode & DEVID_MASK;
    mcu_rev_e revid = (idcode & REVID_MASK) >> 16;

    mcu_des_t des = mcu_descriptions[STM32_UNKNOWN];

    for(size_t i = 0; i < ARR_SIZE(mcu_descriptions); i++) {
        if(mcuid == mcu_descriptions[i].mcuid) {
            des = mcu_descriptions[i];
            break;
        }
    }

    for(size_t i = 0; i < ARR_SIZE(silicon_revs); i++) {
        if(silicon_revs[i].revid == revid) {
            des.rev = silicon_revs[i].rev;
        }
    }

    uint8_t* endp = &revstr[max - 1];
    uint8_t* strp = revstr;

    while(strp < endp && *des.desc) {
        *strp++ = *des.desc++;
    }

    if(strp < endp) {
        *strp++ = ' ';
        *strp++ = 'r';
        *strp++ = 'e';
        *strp++ = 'v';
        *strp++ = '.';
        *strp++ = ' ';
    }

    if(strp < endp) {
        *strp++ = des.rev;
        *strp++ = '\0';
    }

    return strp - revstr;
}

void bsp_mcu_info(void)
{
    uint32_t cpuid = SCB->CPUID;
    uint32_t var, pat;
    

    log_d(TAG, "Core Information:");
    log_d(TAG,
          "CPUID %08lX DEVID %03lX REVID %04lX",
          cpuid,
          DBGMCU->IDCODE & 0xFFF,
          (DBGMCU->IDCODE >> 16) & 0xFFFF);

    pat = (cpuid & 0x0000000F);
    var = (cpuid & 0x00F00000) >> 20;

    if((cpuid & 0xFF000000) == 0x41000000) // ARM
    {
        switch((cpuid & 0x0000FFF0) >> 4) {
        case 0xC20:
            log_d(TAG, "Cortex M0 r%ldp%ld", var, pat);
            break;
        case 0xC60:
            log_d(TAG, "Cortex M0+ r%ldp%ld", var, pat);
            break;
        case 0xC21:
            log_d(TAG, "Cortex M1 r%ldp%ld", var, pat);
            break;
        case 0xC23:
            log_d(TAG, "Cortex M3 r%ldp%ld", var, pat);
            break;
        case 0xC24:
            log_d(TAG, "Cortex M4 r%ldp%ld", var, pat);
            break;
        case 0xC27: {
            
            log_d(TAG, "Cortex M7 r%ldp%ld", var, pat);
            const char mcu_rev[24];
            bsp_mcu_get_mcu_descr(24, (uint8_t*)mcu_rev);
            log_d(TAG, "MCU: %s", mcu_rev);
            break;
        
        }

        default:
            log_d(TAG, "Unknown CORE");
        }
    } else
        log_d(TAG, "Unknown CORE IMPLEMENTER");

    // sys info
    uint32_t hal_ver = HAL_GetHalVersion();
    uint8_t hal_ver_code = ((uint8_t)(hal_ver)) & 0x0F;
    uint16_t* flash_size = (uint16_t*)(FLASHSIZE_BASE);

    log_d(TAG,
          "HAL Version: %d.%d.%d%c",
          ((uint8_t)(hal_ver >> 24)) & 0x0F,
          ((uint8_t)(hal_ver >> 16)) & 0x0F,
          ((uint8_t)(hal_ver >> 8)) & 0x0F,
          hal_ver_code > 0 ? (char)hal_ver_code : ' ');
    log_d(TAG, "Device ID: 0x%lX", HAL_GetDEVID());
    log_d(TAG, "Revision ID: 0x%lX", HAL_GetREVID());
    log_d(TAG, "Flash size: %dk", *flash_size);
    log_d(TAG, "SysClock: %ldMHz", HAL_RCC_GetSysClockFreq() / 1000000);
    log_d(TAG,
          "Unique ID: %08lX%08lX%08lX",
          __builtin_bswap32(HAL_GetUIDw0()),
          __builtin_bswap32(HAL_GetUIDw1()),
          __builtin_bswap32(HAL_GetUIDw2()));

    uint32_t mvfr0;

    log_d(TAG,
          "%08lX %08lX %08lX",
          *(volatile uint32_t*)0xE000EF34,  // FPCCR  0xC0000000
          *(volatile uint32_t*)0xE000EF38,  // FPCAR
          *(volatile uint32_t*)0xE000EF3C); // FPDSCR

    log_d(TAG,
          "%08lX %08lX %08lX",
          *(volatile uint32_t*)0xE000EF40,  // MVFR0  0x10110021 vs 0x10110221
          *(volatile uint32_t*)0xE000EF44,  // MVFR1  0x11000011 vs 0x12000011
          *(volatile uint32_t*)0xE000EF48); // MVFR2  0x00000040

    mvfr0 = *(volatile uint32_t*)0xE000EF40;

    switch(mvfr0) {
    case 0x00000000:
        log_w(TAG, "No FPU");
        break;
    case 0x10110021:
        log_d(TAG, "FPU-S Single-precision only FPU");
        break;
    case 0x10110221:
        log_d(TAG, "FPU-D Single-precision and Double-precision FPU");
        break;
    default:
        log_e(TAG, "Unknown FPU");
    }

}