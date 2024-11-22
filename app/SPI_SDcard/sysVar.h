#ifndef __SYSVAR_H
#define __SYSVAR_H

#include "main.h"
#include "stm32f4xx.h"

#define SPI2_CS_H HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET);
#define SPI2_CS_L HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET);

#endif