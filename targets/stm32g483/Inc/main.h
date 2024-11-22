/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

#include "stm32g4xx_ll_adc.h"
#include "stm32g4xx_ll_cordic.h"
#include "stm32g4xx_ll_dac.h"
#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_bus.h"
#include "stm32g4xx_ll_crs.h"
#include "stm32g4xx_ll_system.h"
#include "stm32g4xx_ll_exti.h"
#include "stm32g4xx_ll_cortex.h"
#include "stm32g4xx_ll_utils.h"
#include "stm32g4xx_ll_pwr.h"
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_tim.h"
#include "stm32g4xx_ll_gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "arm_math.h"
#include "stm32g4xx.h"
#include "stm32g4xx_it.h"
#include "foc.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
//void autodetect(foc_t *pf); 
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CS11_Pin LL_GPIO_PIN_9
#define CS11_GPIO_Port GPIOB
#define CS12_Pin LL_GPIO_PIN_3
#define CS12_GPIO_Port GPIOE
#define CS22_Pin LL_GPIO_PIN_4
#define CS22_GPIO_Port GPIOE
#define CS21_Pin LL_GPIO_PIN_13
#define CS21_GPIO_Port GPIOC
#define ENABLE_Pin LL_GPIO_PIN_14
#define ENABLE_GPIO_Port GPIOD
#define FAULT_RESET_Pin LL_GPIO_PIN_15
#define FAULT_RESET_GPIO_Port GPIOD
#define LED_RED_Pin LL_GPIO_PIN_9
#define LED_RED_GPIO_Port GPIOC
#define RS_REDE_Pin LL_GPIO_PIN_12
#define RS_REDE_GPIO_Port GPIOA
#define LED_YELLOW_Pin LL_GPIO_PIN_10
#define LED_YELLOW_GPIO_Port GPIOC
#define LED_GREEN_Pin LL_GPIO_PIN_11
#define LED_GREEN_GPIO_Port GPIOC
#define ALARM_U_Pin LL_GPIO_PIN_5
#define ALARM_U_GPIO_Port GPIOB
#define ALARM_V_Pin LL_GPIO_PIN_6
#define ALARM_V_GPIO_Port GPIOB
#define ALARM_W_Pin LL_GPIO_PIN_7
#define ALARM_W_GPIO_Port GPIOB
#define SPI2_CS1_Pin LL_GPIO_PIN_9
#define SPI2_CS1_GPIO_Port GPIOB
#define IES1_Pin LL_GPIO_PIN_0
#define IES1_GPIO_Port GPIOE
#define IES2_Pin LL_GPIO_PIN_1
#define IES2_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */
#define ENABLE_PIN_SET    HAL_GPIO_WritePin(ENABLE_GPIO_Port, ENABLE_Pin, GPIO_PIN_SET);
#define ENABLE_PIN_RESET  HAL_GPIO_WritePin(ENABLE_GPIO_Port, ENABLE_Pin, GPIO_PIN_RESET);
#define FAULT_RESET       HAL_GPIO_WritePin(FAULT_RESET_GPIO_Port, FAULT_RESET_Pin, GPIO_PIN_RESET);
#define FAULT_RESET_SET   HAL_GPIO_WritePin(FAULT_RESET_GPIO_Port, FAULT_RESET_Pin, GPIO_PIN_SET);

#define iabs(x) (((x) >= 0)?(x):-(x))
#define sign(x) (((x) >= 0)?(1):(-1))

#define MAIN_LOW_PERIOD_US 1000
#define _U_MAX  5280  //little lower than period of timer1 for proper phase current reading
#define _T      5312   //2600 - 24V motor ; 5312 - 48V motor
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
