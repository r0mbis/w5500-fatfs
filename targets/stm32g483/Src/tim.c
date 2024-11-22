/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances.
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
/* Includes ------------------------------------------------------------------*/
#include "tim.h"

/* USER CODE BEGIN 0 */
void tim1_start(volatile uint32_t pwmFreq, volatile uint32_t deadtime, volatile uint32_t *halfPeriod)
{
  *halfPeriod = (170000000 / pwmFreq) / 4;
  LL_TIM_SetAutoReload(TIM1, (uint32_t)(*halfPeriod * 2));
  LL_TIM_OC_SetDeadTime(TIM1, deadtime);
  LL_TIM_OC_SetCompareCH4(TIM1, (uint32_t)(*halfPeriod * 2 - 5));

  /* Set duty to 0 */
  // LL_TIM_OC_SetCompareCH1(TIM1, 0);
  // LL_TIM_OC_SetCompareCH2(TIM1, 0);
  // LL_TIM_OC_SetCompareCH3(TIM1, 0);
  
  // LL_TIM_EnableIT_UPDATE(TIM1);
  // LL_TIM_EnableIT_CC1(TIM1);
  // LL_TIM_EnableIT_CC2(TIM1);
  // LL_TIM_EnableIT_CC3(TIM1);
  // LL_TIM_EnableIT_CC4(TIM1);
  LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1);
  LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1N);
  LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH2);
  LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH2N);
  LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH3);
  LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH3N);
  LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH4);
  LL_TIM_EnableCounter(TIM1);
  LL_TIM_EnableAllOutputs(TIM1);  //??
}


void tim8_start(void)
{
  LL_TIM_EnableIT_CC1(TIM8);
  //LL_TIM_ClearFlag_UPDATE(TIM8);
  //LL_TIM_EnableIT_UPDATE(TIM8);
  LL_TIM_CC_EnableChannel(TIM8, LL_TIM_CHANNEL_CH1);
  LL_TIM_EnableCounter(TIM8);
}

void tim3_start(void)
{
  LL_TIM_EnableIT_UPDATE(TIM3);
  LL_TIM_EnableCounter(TIM3);
}

void tim7_start(void)
{
  //LL_TIM_EnableIT_UPDATE(TIM7);
  LL_TIM_EnableCounter(TIM7);
}
/* USER CODE END 0 */

/* TIM1 init function */
void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
  LL_TIM_BDTR_InitTypeDef TIM_BDTRInitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  TIM_InitStruct.Prescaler = 0;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_CENTER_DOWN;
  TIM_InitStruct.Autoreload = 5312;    //5312 - 16khz
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  TIM_InitStruct.RepetitionCounter = 0;
  LL_TIM_Init(TIM1, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM1);
  LL_TIM_SetClockSource(TIM1, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH1);
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.CompareValue = 2656;  //2656; 1325
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  TIM_OC_InitStruct.OCNPolarity = LL_TIM_OCPOLARITY_HIGH;
  TIM_OC_InitStruct.OCIdleState = LL_TIM_OCIDLESTATE_LOW;
  TIM_OC_InitStruct.OCNIdleState = LL_TIM_OCIDLESTATE_LOW;
  LL_TIM_OC_Init(TIM1, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM1, LL_TIM_CHANNEL_CH1);
  LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH2);
  LL_TIM_OC_Init(TIM1, LL_TIM_CHANNEL_CH2, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM1, LL_TIM_CHANNEL_CH2);
  LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH3);
  LL_TIM_OC_Init(TIM1, LL_TIM_CHANNEL_CH3, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM1, LL_TIM_CHANNEL_CH3);
  LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH4);

  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM2;
  TIM_OC_InitStruct.CompareValue = (5312 - 4); // (2656-4)
  LL_TIM_OC_Init(TIM1, LL_TIM_CHANNEL_CH4, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM1, LL_TIM_CHANNEL_CH4);
  LL_TIM_SetTriggerOutput(TIM1, LL_TIM_TRGO_OC4REF);
  LL_TIM_SetTriggerOutput2(TIM1, LL_TIM_TRGO2_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM1);
  TIM_BDTRInitStruct.OSSRState = LL_TIM_OSSR_ENABLE;
  TIM_BDTRInitStruct.OSSIState = LL_TIM_OSSI_ENABLE;
  TIM_BDTRInitStruct.LockLevel = LL_TIM_LOCKLEVEL_OFF;
  TIM_BDTRInitStruct.DeadTime = 127;
  TIM_BDTRInitStruct.BreakState = LL_TIM_BREAK_DISABLE;
  TIM_BDTRInitStruct.BreakPolarity = LL_TIM_BREAK_POLARITY_HIGH;
  TIM_BDTRInitStruct.BreakFilter = LL_TIM_BREAK_FILTER_FDIV1_N8;
  TIM_BDTRInitStruct.BreakAFMode = LL_TIM_BREAK_AFMODE_INPUT;
  TIM_BDTRInitStruct.Break2State = LL_TIM_BREAK2_DISABLE;
  TIM_BDTRInitStruct.Break2Polarity = LL_TIM_BREAK2_POLARITY_HIGH;
  TIM_BDTRInitStruct.Break2Filter = LL_TIM_BREAK2_FILTER_FDIV1_N8;
  TIM_BDTRInitStruct.Break2AFMode = LL_TIM_BREAK_AFMODE_INPUT;
  TIM_BDTRInitStruct.AutomaticOutput = LL_TIM_AUTOMATICOUTPUT_DISABLE;
  LL_TIM_BDTR_Init(TIM1, &TIM_BDTRInitStruct);
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOE);
    /**TIM1 GPIO Configuration
    PE8     ------> TIM1_CH1N
    PE9     ------> TIM1_CH1
    PE10     ------> TIM1_CH2N
    PE11     ------> TIM1_CH2
    PE12     ------> TIM1_CH3N
    PE13     ------> TIM1_CH3
    */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_8;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_11;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_12;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_13;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(GPIOE, &GPIO_InitStruct);

}

/* TIM8 init function */
void MX_TIM8_Init(void)
{

  /* USER CODE BEGIN TIM8_Init 0 */

  /* USER CODE END TIM8_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM8);

  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
  /**TIM8 GPIO Configuration
  PC6   ------> TIM8_CH1
  PC7   ------> TIM8_CH2
  PC8   ------> TIM8_CH3
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_8;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM8_Init 1 */
  /* TIM8 interrupt Init */
  NVIC_SetPriority(TIM8_CC_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),2, 0));
  NVIC_EnableIRQ(TIM8_CC_IRQn);
  NVIC_EnableIRQ(TIM8_UP_IRQn);
  /* USER CODE END TIM8_Init 1 */
  TIM_InitStruct.Prescaler = 339;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 65535;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  TIM_InitStruct.RepetitionCounter = 0;
  LL_TIM_Init(TIM8, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM8);
  LL_TIM_SetClockSource(TIM8, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_SetTriggerInput(TIM8, LL_TIM_TS_TI1F_ED);
  LL_TIM_SetSlaveMode(TIM8, LL_TIM_SLAVEMODE_RESET);
  LL_TIM_CC_DisableChannel(TIM8, LL_TIM_CHANNEL_CH1);
  LL_TIM_IC_SetFilter(TIM8, LL_TIM_CHANNEL_CH1, LL_TIM_IC_FILTER_FDIV1);
  LL_TIM_DisableIT_TRIG(TIM8);
  LL_TIM_DisableDMAReq_TRIG(TIM8);
  LL_TIM_SetTriggerOutput(TIM8, LL_TIM_TRGO_RESET);
  LL_TIM_SetTriggerOutput2(TIM8, LL_TIM_TRGO2_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM8);
  LL_TIM_IC_SetActiveInput(TIM8, LL_TIM_CHANNEL_CH1, LL_TIM_ACTIVEINPUT_TRC);
  LL_TIM_IC_SetPrescaler(TIM8, LL_TIM_CHANNEL_CH1, LL_TIM_ICPSC_DIV1);
  LL_TIM_IC_SetFilter(TIM8, LL_TIM_CHANNEL_CH1, LL_TIM_IC_FILTER_FDIV32_N8);
  LL_TIM_IC_SetPolarity(TIM8, LL_TIM_CHANNEL_CH1, LL_TIM_IC_POLARITY_RISING); //проверить на FALLING
  LL_TIM_IC_SetActiveInput(TIM8, LL_TIM_CHANNEL_CH2, LL_TIM_ACTIVEINPUT_DIRECTTI);
  LL_TIM_IC_SetPrescaler(TIM8, LL_TIM_CHANNEL_CH2, LL_TIM_ICPSC_DIV1);
  LL_TIM_IC_SetFilter(TIM8, LL_TIM_CHANNEL_CH2, LL_TIM_IC_FILTER_FDIV32_N8);
  LL_TIM_IC_SetPolarity(TIM8, LL_TIM_CHANNEL_CH2, LL_TIM_IC_POLARITY_RISING);
  LL_TIM_IC_SetActiveInput(TIM8, LL_TIM_CHANNEL_CH3, LL_TIM_ACTIVEINPUT_DIRECTTI);
  LL_TIM_IC_SetPrescaler(TIM8, LL_TIM_CHANNEL_CH3, LL_TIM_ICPSC_DIV1);
  LL_TIM_IC_SetFilter(TIM8, LL_TIM_CHANNEL_CH3, LL_TIM_IC_FILTER_FDIV32_N8);
  LL_TIM_IC_SetPolarity(TIM8, LL_TIM_CHANNEL_CH3, LL_TIM_IC_POLARITY_RISING);
  LL_TIM_IC_EnableXORCombination(TIM8);
}
  /* USER CODE BEGIN TIM8_Init 2 */
/* TIM3 init function */
/*8khz timer*/
void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

  /* TIM3 interrupt Init */
  NVIC_SetPriority(TIM3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 3, 0));
  NVIC_EnableIRQ(TIM3_IRQn);

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  TIM_InitStruct.Prescaler = 0;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 21249;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM3, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM3);
  LL_TIM_SetClockSource(TIM3, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_SetTriggerOutput(TIM3, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM3);
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
}
/*1ms timer*/
void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM7);

  /* TIM7 interrupt Init */
  // NVIC_SetPriority(TIM7_DAC_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
  // NVIC_EnableIRQ(TIM7_DAC_IRQn);

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  TIM_InitStruct.Prescaler = 169;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 65535; // 999
  LL_TIM_Init(TIM7, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM7);
  LL_TIM_SetTriggerOutput(TIM7, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM7);
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}
  /* USER CODE END TIM8_Init 2 */



/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
