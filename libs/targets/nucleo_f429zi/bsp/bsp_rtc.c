#include "bsp_rtc.h"

#include "sys_log.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_rtc.h"
// #include "stm32f4xx_hal_rtc_ex.h"
#include "rtc.h"

static volatile uint8_t __rtc_init_complete = 0;
extern RTC_HandleTypeDef hrtc;

/* RTC init function */
void bsp_rtc_init(void) {

    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /** Initialize RTC Only
    */
    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 255;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    // hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
    if(HAL_RTC_Init(&hrtc) != HAL_OK) {
        // Error_Handler();
        log_e("RTC", "Failed to init");
    }

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
    if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        log_e("RTC", "Failed to periph clock");
    }

    __rtc_init_complete = 1;
}

uint32_t bsp_rtc_get_signature(void) {
    /* enable the backup registers */
    if(!__rtc_init_complete) {
        bsp_rtc_init();
    }

    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_RTC_ENABLE();
    uint32_t result = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0);
    /* disable the backup registers */
    __HAL_RCC_RTC_DISABLE();
    HAL_PWR_DisableBkUpAccess();

    HAL_RTC_DeInit(&hrtc);

    return result;
}

void bsp_rtc_set_signature(uint32_t sig) {
    // 2do init just here for now as only used before system reset
    if(!__rtc_init_complete) {
        bsp_rtc_init();
    }
    
    // __disable_irq();

    /* enable the backup registers */
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_RTC_ENABLE();
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, sig);
    /* disable the backup registers */
    __HAL_RCC_RTC_DISABLE();
    HAL_PWR_DisableBkUpAccess();
    HAL_RTC_DeInit(&hrtc);
    // __enable_irq();
}