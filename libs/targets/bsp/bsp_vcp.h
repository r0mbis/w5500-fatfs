#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "usart.h"

#define SIZE_OF_TX_POOL 128
#define SIZE_OF_TX_BUFFER 64
#define SIZE_OF_RX_CIRCULAR_BUFFER 2048

typedef struct
{
	uint8_t data[SIZE_OF_TX_BUFFER];
	uint32_t length;
} tx_buffer;


extern USART_TypeDef* HAL_VCP_UART;
extern UART_HandleTypeDef* bsp_uart_handler;

void bsp_vcp_uart_init(void);
void bsp_vcp_uart_deinit(void);
void bsp_vcp_tx(char* c);
int bps_vcp_available();
void bsp_vcp_start_it();
uint32_t bsp_vcp_rx_it();

#ifdef __cplusplus
}
#endif
