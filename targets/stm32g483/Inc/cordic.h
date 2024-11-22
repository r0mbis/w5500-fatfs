/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    cordic.h
  * @brief   This file contains all the function prototypes for
  *          the cordic.c file
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
#ifndef __CORDIC_H__
#define __CORDIC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "utils.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_CORDIC_Init(void);

/* USER CODE BEGIN Prototypes */

  /** Calc sinCos using hardware CORDIC
   *  Input: angle in radians (range -pi : pi)
   *  Output: sin, cos (range -1 : 1)
   *  Average execution time: ~98 CPU cycles
   **/
  static inline void cordic_sincos_calc(volatile float angle, volatile float *s, volatile float *c)
  {
    if (LL_CORDIC_GetFunction(CORDIC) != LL_CORDIC_FUNCTION_SINE)
    {
      /* Force CORDIC reset */
      LL_AHB1_GRP1_ForceReset(LL_AHB1_GRP1_PERIPH_CORDIC);
      /* Release CORDIC reset */
      LL_AHB1_GRP1_ReleaseReset(LL_AHB1_GRP1_PERIPH_CORDIC);

      LL_CORDIC_Config(CORDIC, LL_CORDIC_FUNCTION_SINE, /* cosine function */
                       LL_CORDIC_PRECISION_6CYCLES,     /* max precision for q1.31 cosine */
                       LL_CORDIC_SCALE_0,               /* no scale */
                       LL_CORDIC_NBWRITE_1,             /* One input data: angle. Second input data (modulus) is 1 after cordic reset */
                       LL_CORDIC_NBREAD_2,              /* Two output data: sine, then cosine */
                       LL_CORDIC_INSIZE_32BITS,         /* q1.31 format for input data */
                       LL_CORDIC_OUTSIZE_32BITS);       /* q1.31 format for output data */
    }

    float angle_wrap = WRAP_TO_PI(angle);
    LL_CORDIC_WriteData(CORDIC, _IQ31(angle_wrap * ONE_BY_PI)); // cordic input data format q31, angle range (1 : -1)
    *s = _IQ31toF((int32_t)LL_CORDIC_ReadData(CORDIC));
    *c = _IQ31toF((int32_t)LL_CORDIC_ReadData(CORDIC));
  }
  
  static inline void cordic_sincos_calc_q31(volatile int32_t angle, volatile int32_t *s, volatile int32_t *c)
  {
    if (LL_CORDIC_GetFunction(CORDIC) != LL_CORDIC_FUNCTION_SINE)
    {
      /* Force CORDIC reset */
      LL_AHB1_GRP1_ForceReset(LL_AHB1_GRP1_PERIPH_CORDIC);
      /* Release CORDIC reset */
      LL_AHB1_GRP1_ReleaseReset(LL_AHB1_GRP1_PERIPH_CORDIC);

      LL_CORDIC_Config(CORDIC, LL_CORDIC_FUNCTION_SINE, /* cosine function */
                       LL_CORDIC_PRECISION_6CYCLES,     /* max precision for q1.31 cosine */
                       LL_CORDIC_SCALE_0,               /* no scale */
                       LL_CORDIC_NBWRITE_1,             /* One input data: angle. Second input data (modulus) is 1 after cordic reset */
                       LL_CORDIC_NBREAD_2,              /* Two output data: sine, then cosine */
                       LL_CORDIC_INSIZE_32BITS,         /* q1.31 format for input data */
                       LL_CORDIC_OUTSIZE_32BITS);       /* q1.31 format for output data */
    }

    LL_CORDIC_WriteData(CORDIC, angle); // cordic input data format q31, angle range (1 : -1)
    *s = (int32_t)LL_CORDIC_ReadData(CORDIC);
    *c = (int32_t)LL_CORDIC_ReadData(CORDIC);
  }

  /** Calc atan2 using hardware CORDIC
   * Input: x, y (range -1 : 1)
   * Output: angle in radians (range -pi : pi)
   **/
  static inline void cordic_atan2_calc(volatile float x, volatile float y, volatile float *angle)
  {
    if (LL_CORDIC_GetFunction(CORDIC) != LL_CORDIC_FUNCTION_PHASE)
    {
      /* Force CORDIC reset */
      LL_AHB1_GRP1_ForceReset(LL_AHB1_GRP1_PERIPH_CORDIC);
      /* Release CORDIC reset */
      LL_AHB1_GRP1_ReleaseReset(LL_AHB1_GRP1_PERIPH_CORDIC);

      LL_CORDIC_Config(CORDIC, LL_CORDIC_FUNCTION_PHASE, /* phase/atan2 function */
                       LL_CORDIC_PRECISION_6CYCLES,      /* max precision for q1.31 cosine */
                       LL_CORDIC_SCALE_0,                /* no scale */
                       LL_CORDIC_NBWRITE_2,              /* Two input data: sin, cos */
                       LL_CORDIC_NBREAD_1,               /* One output data: phase */
                       LL_CORDIC_INSIZE_32BITS,          /* q1.31 format for input data */
                       LL_CORDIC_OUTSIZE_32BITS);        /* q1.31 format for output data */
    }

    LL_CORDIC_WriteData(CORDIC, _IQ31(x));
    LL_CORDIC_WriteData(CORDIC, _IQ31(y));
    *angle = _IQ31toF((int32_t)LL_CORDIC_ReadData(CORDIC)) * MF_PI;
  }
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CORDIC_H__ */

