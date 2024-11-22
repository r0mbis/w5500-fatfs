#ifndef W5500_DRIVER_H
#define W5500_DRIVER_H

#include "main.h"
#include <stdbool.h>
#include "wizchip_conf.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "event_groups.h"
//#include "queue.h"

#define DHCP_SOCKET     0
#define DNS_SOCKET      1
#define HTTP_SOCKET     2

extern SemaphoreHandle_t spi_mutex;

static inline void SPI_CrisEnter(void)
{
    vPortEnterCritical();
    //__set_PRIMASK(1);
}

static inline void SPI_CrisExit(void)
{
    vPortExitCritical();
    __set_PRIMASK(0);
}

static inline void SPI_CS_Select(void)
{
    //if (xSemaphoreTake(spi_mutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    //{
        HAL_GPIO_WritePin(SPI1_CS_GPIO_Port,SPI1_CS_Pin,GPIO_PIN_RESET);
    //}
}

static inline void SPI_CS_Deselect(void)
{
    HAL_GPIO_WritePin(SPI1_CS_GPIO_Port,SPI1_CS_Pin,GPIO_PIN_SET);
    //xSemaphoreGive(spi_mutex);
}

void InitW5500(void);
void network_init(void);
void SPI_WriteByte(unsigned char TxData);
unsigned char SPI_ReadByte(void);
void SPI_Writebuff(uint8_t* buff, uint16_t len);
void SPI_Readbuff(uint8_t* buff, uint16_t len);
void print_network_information(wiz_NetInfo net_info);
void dhcp_init(EventGroupHandle_t eventgroup);
void network_initialize(wiz_NetInfo net_info);

#endif