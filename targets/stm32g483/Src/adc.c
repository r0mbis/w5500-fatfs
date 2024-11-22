/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.c
  * @brief   This file provides code for the configuration
  *          of the ADC instances.
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
#include "adc.h"

/* USER CODE BEGIN 0 */
void adc_start(void)
{
  LL_ADC_StartCalibration(ADC1, LL_ADC_SINGLE_ENDED);
  /* Poll for ADC effectively calibrated */
  while (LL_ADC_IsCalibrationOnGoing(ADC1) != 0)
  {
  }
  /* Delay between ADC end of calibration and ADC enable.                   */
  uint32_t wait_loop_index;
  wait_loop_index = (LL_ADC_DELAY_CALIB_ENABLE_ADC_CYCLES * 2);
  while(wait_loop_index != 0)
  {
    wait_loop_index--;
  }
  /* Enable ADC */
  LL_ADC_EnableIT_EOC(ADC1);
  LL_ADC_Enable(ADC1);
  /* Poll for ADC ready to convert */
  while (LL_ADC_IsActiveFlag_ADRDY(ADC1) == 0)
  {
  }
  LL_ADC_ClearFlag_ADRDY(ADC1);
  // p->volt.vdda = measure_Vdda_ADC1();
  // p->config.adcFullScaleCurrent = p->volt.vdda * 36.45833333f / 2.0f;
  // p->config.adcFullScaleVoltageDC = p->volt.vdda * USER_Vbus_divider;
  // p->config.adcFullScaleVoltage = p->volt.vdda * USER_Vphase_divider;
  /* Enable end of sequence convertion ISR */
  //LL_ADC_EnableIT_JEOS(ADC1);

  /* Run ADC self calibration */
  LL_ADC_StartCalibration(ADC2, LL_ADC_SINGLE_ENDED);
  /* Poll for ADC effectively calibrated */
  while (LL_ADC_IsCalibrationOnGoing(ADC2) != 0)
  {
  }
  /* Delay between ADC end of calibration and ADC enable.                   */
  wait_loop_index = (LL_ADC_DELAY_CALIB_ENABLE_ADC_CYCLES * 2);
  while(wait_loop_index != 0)
  {
    wait_loop_index--;
  }
  /* Enable ADC */
  //LL_ADC_EnableIT_EOC(ADC2);
  LL_ADC_Enable(ADC2);
  /* Poll for ADC ready to convert */
  while (LL_ADC_IsActiveFlag_ADRDY(ADC2) == 0)
  {
  }
  LL_ADC_ClearFlag_ADRDY(ADC2);
  /* Enable end of convertion ISR */
  //LL_ADC_EnableIT_JEOC(ADC2);
  /* Run ADC self calibration */
  LL_ADC_StartCalibration(ADC3, LL_ADC_SINGLE_ENDED);
  /* Poll for ADC effectively calibrated */
  while (LL_ADC_IsCalibrationOnGoing(ADC3) != 0)
  {
  }
  /* Delay between ADC end of calibration and ADC enable.                   */
  wait_loop_index = (LL_ADC_DELAY_CALIB_ENABLE_ADC_CYCLES * 2);
  while(wait_loop_index != 0)
  {
    wait_loop_index--;
  }
  /* Enable ADC */
  //LL_ADC_EnableIT_EOC(ADC2);
  LL_ADC_Enable(ADC3);
  /* Poll for ADC ready to convert */
  while (LL_ADC_IsActiveFlag_ADRDY(ADC3) == 0)
  {
  }
  LL_ADC_ClearFlag_ADRDY(ADC3);

  /* auto-inject mode convertions will start after ADSTART->CR (not JADSTART) */
  //LL_ADC_INJ_StartConversion(ADC1);
  //LL_ADC_INJ_StartConversion(ADC2);
  LL_ADC_REG_StartConversion(ADC3);
  LL_ADC_REG_StartConversion(ADC2);
  LL_ADC_REG_StartConversion(ADC1);
  
}
/* USER CODE END 0 */

/* ADC1 init function */
void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  LL_ADC_InitTypeDef ADC_InitStruct = {0};
  LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};
  LL_ADC_CommonInitTypeDef ADC_CommonInitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_RCC_SetADCClockSource(LL_RCC_ADC12_CLKSOURCE_SYSCLK);

  /* Peripheral clock enable */
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_ADC12);

  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
  /**ADC1 GPIO Configuration
  PB0   ------> ADC1_IN15 (PHASE_U)
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_0;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* ADC1 interrupt Init */
  NVIC_SetPriority(ADC1_2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(ADC1_2_IRQn);

  /* USER CODE BEGIN ADC1_Init 1 */
  LL_ADC_DisableIT_EOC(ADC1);
  LL_ADC_DisableIT_EOS(ADC1);
  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B;
  ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
  ADC_InitStruct.LowPowerMode = LL_ADC_LP_MODE_NONE;
  LL_ADC_Init(ADC1, &ADC_InitStruct);
  ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_EXT_TIM1_TRGO;
  ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE;
  ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
  ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
  ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_NONE;
  ADC_REG_InitStruct.Overrun = LL_ADC_REG_OVR_DATA_OVERWRITTEN;
  LL_ADC_REG_Init(ADC1, &ADC_REG_InitStruct);
  LL_ADC_SetGainCompensation(ADC1, 0);
  LL_ADC_SetOverSamplingScope(ADC1, LL_ADC_OVS_DISABLE);
  ADC_CommonInitStruct.CommonClock = LL_ADC_CLOCK_SYNC_PCLK_DIV2; //LL_ADC_CLOCK_SYNC_PCLK_DIV4
  ADC_CommonInitStruct.Multimode = LL_ADC_MULTI_DUAL_REG_SIMULT;
  ADC_CommonInitStruct.MultiDMATransfer = LL_ADC_MULTI_REG_DMA_EACH_ADC;
  ADC_CommonInitStruct.MultiTwoSamplingDelay = LL_ADC_MULTI_TWOSMP_DELAY_1CYCLE;
  LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(ADC1), &ADC_CommonInitStruct);
  LL_ADC_REG_SetTriggerEdge(ADC1, LL_ADC_REG_TRIG_EXT_RISING);

  /* Disable ADC deep power down (enabled by default after reset state) */
  LL_ADC_DisableDeepPowerDown(ADC1);
  /* Enable ADC internal voltage regulator */
  LL_ADC_EnableInternalRegulator(ADC1);
  /* Delay for ADC internal voltage regulator stabilization. */
  /* Compute number of CPU cycles to wait for, from delay in us. */
  /* Note: Variable divided by 2 to compensate partially */
  /* CPU processing cycles (depends on compilation optimization). */
  /* Note: If system core clock frequency is below 200kHz, wait time */
  /* is only a few CPU processing cycles. */
  uint32_t wait_loop_index;
  wait_loop_index = ((LL_ADC_DELAY_INTERNAL_REGUL_STAB_US * (SystemCoreClock / (100000 * 2))) / 10);
  while(wait_loop_index != 0)
  {
    wait_loop_index--;
  }

  /** Configure Regular Channel
  */
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_15);
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_15, LL_ADC_SAMPLINGTIME_6CYCLES_5);
  LL_ADC_SetChannelSingleDiff(ADC1, LL_ADC_CHANNEL_15, LL_ADC_SINGLE_ENDED);
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}
/* ADC2 init function */
void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  LL_ADC_InitTypeDef ADC_InitStruct = {0};
  LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_RCC_SetADCClockSource(LL_RCC_ADC12_CLKSOURCE_SYSCLK);

  /* Peripheral clock enable */
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_ADC12);

  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
  /**ADC2 GPIO Configuration
  PB2   ------> ADC2_IN12 (PHASE_W)
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* ADC2 interrupt Init */
  NVIC_SetPriority(ADC1_2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(ADC1_2_IRQn);

  /* USER CODE BEGIN ADC2_Init 1 */
  LL_ADC_DisableIT_EOC(ADC2);
  LL_ADC_DisableIT_EOS(ADC2);
  /* USER CODE END ADC2_Init 1 */

  /** Common config
  */
  ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B;
  ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
  ADC_InitStruct.LowPowerMode = LL_ADC_LP_MODE_NONE;
  LL_ADC_Init(ADC2, &ADC_InitStruct);
  ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE;
  ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
  ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
  ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_NONE;
  ADC_REG_InitStruct.Overrun = LL_ADC_REG_OVR_DATA_OVERWRITTEN;
  LL_ADC_REG_Init(ADC2, &ADC_REG_InitStruct);
  LL_ADC_SetGainCompensation(ADC2, 0);
  LL_ADC_SetOverSamplingScope(ADC2, LL_ADC_OVS_DISABLE);

  /* Disable ADC deep power down (enabled by default after reset state) */
  LL_ADC_DisableDeepPowerDown(ADC2);
  /* Enable ADC internal voltage regulator */
  LL_ADC_EnableInternalRegulator(ADC2);
  /* Delay for ADC internal voltage regulator stabilization. */
  /* Compute number of CPU cycles to wait for, from delay in us. */
  /* Note: Variable divided by 2 to compensate partially */
  /* CPU processing cycles (depends on compilation optimization). */
  /* Note: If system core clock frequency is below 200kHz, wait time */
  /* is only a few CPU processing cycles. */
  uint32_t wait_loop_index;
  wait_loop_index = ((LL_ADC_DELAY_INTERNAL_REGUL_STAB_US * (SystemCoreClock / (100000 * 2))) / 10);
  while(wait_loop_index != 0)
  {
    wait_loop_index--;
  }

  /** Configure Regular Channel
  */
  LL_ADC_REG_SetSequencerRanks(ADC2, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_12);
  LL_ADC_SetChannelSamplingTime(ADC2, LL_ADC_CHANNEL_12, LL_ADC_SAMPLINGTIME_6CYCLES_5);
  LL_ADC_SetChannelSingleDiff(ADC2, LL_ADC_CHANNEL_12, LL_ADC_SINGLE_ENDED);
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}
/* ADC3 init function */
void MX_ADC3_Init(void)
{

  /* USER CODE BEGIN ADC3_Init 0 */

  /* USER CODE END ADC3_Init 0 */

  LL_ADC_InitTypeDef ADC_InitStruct = {0};
  LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};
  LL_ADC_CommonInitTypeDef ADC_CommonInitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_RCC_SetADCClockSource(LL_RCC_ADC345_CLKSOURCE_SYSCLK);

  /* Peripheral clock enable */
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_ADC345);

  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
  /**ADC3 GPIO Configuration
  PB1   ------> ADC3_IN1 (PHASE_V)
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_1;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN ADC3_Init 1 */
  LL_ADC_DisableIT_EOC(ADC3);
  LL_ADC_DisableIT_EOS(ADC3);
  /* USER CODE END ADC3_Init 1 */

  /** Common config
  */
  ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B;
  ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
  ADC_InitStruct.LowPowerMode = LL_ADC_LP_MODE_NONE;
  LL_ADC_Init(ADC3, &ADC_InitStruct);
  ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_EXT_TIM1_TRGO;
  ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE;
  ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
  ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
  ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_NONE;
  ADC_REG_InitStruct.Overrun = LL_ADC_REG_OVR_DATA_OVERWRITTEN;
  LL_ADC_REG_Init(ADC3, &ADC_REG_InitStruct);
  LL_ADC_SetGainCompensation(ADC3, 0);
  LL_ADC_SetOverSamplingScope(ADC3, LL_ADC_OVS_DISABLE);
  ADC_CommonInitStruct.CommonClock = LL_ADC_CLOCK_SYNC_PCLK_DIV2; //LL_ADC_CLOCK_SYNC_PCLK_DIV4
  ADC_CommonInitStruct.Multimode = LL_ADC_MULTI_INDEPENDENT;
  LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(ADC3), &ADC_CommonInitStruct);
  LL_ADC_REG_SetTriggerEdge(ADC3, LL_ADC_REG_TRIG_EXT_RISING);

  /* Disable ADC deep power down (enabled by default after reset state) */
  LL_ADC_DisableDeepPowerDown(ADC3);
  /* Enable ADC internal voltage regulator */
  LL_ADC_EnableInternalRegulator(ADC3);
  /* Delay for ADC internal voltage regulator stabilization. */
  /* Compute number of CPU cycles to wait for, from delay in us. */
  /* Note: Variable divided by 2 to compensate partially */
  /* CPU processing cycles (depends on compilation optimization). */
  /* Note: If system core clock frequency is below 200kHz, wait time */
  /* is only a few CPU processing cycles. */
  uint32_t wait_loop_index;
  wait_loop_index = ((LL_ADC_DELAY_INTERNAL_REGUL_STAB_US * (SystemCoreClock / (100000 * 2))) / 10);
  while(wait_loop_index != 0)
  {
    wait_loop_index--;
  }

  /** Configure Regular Channel
  */
  LL_ADC_REG_SetSequencerRanks(ADC3, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_1);
  LL_ADC_SetChannelSamplingTime(ADC3, LL_ADC_CHANNEL_1, LL_ADC_SAMPLINGTIME_6CYCLES_5);
  LL_ADC_SetChannelSingleDiff(ADC3, LL_ADC_CHANNEL_1, LL_ADC_SINGLE_ENDED);
  /* USER CODE BEGIN ADC3_Init 2 */

  /* USER CODE END ADC3_Init 2 */

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
