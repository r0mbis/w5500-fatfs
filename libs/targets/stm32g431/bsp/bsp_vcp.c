#include "bsp_vcp.h"
#include "usart.h"

static uint8_t UARTaRxBuffer[10];

USART_TypeDef* HAL_VCP_UART = ((USART_TypeDef *) USART2_BASE);

UART_HandleTypeDef* bsp_uart_handler = &huart2;



void bsp_vcp_uart_init(void) {
    MX_USART2_UART_Init();
}

void bsp_vcp_uart_deinit(void) {
    HAL_UART_DeInit(bsp_uart_handler);
}

void bsp_vcp_tx(char* c){
    while(!(HAL_VCP_UART->ISR & USART_ISR_TXE_TXFNF)) {
    };

    HAL_VCP_UART->TDR = (uint8_t)(*c & 0xFFU); // 16b -> 8b
    // HAL_VCP_UART->TDR = *c;


    // HAL_UART_Transmit(bsp_uart_handler, (const uint8_t*) c, 1, 100);
}



void bsp_vcp_start_it(){
    if(HAL_UART_Receive_IT(bsp_uart_handler, (uint8_t*)UARTaRxBuffer, 10) != HAL_OK) {
        bsp_uart_handler->gState = HAL_UART_STATE_READY;
    }
}


uint32_t bsp_vcp_rx_it(){
  while(!(HAL_VCP_UART->ISR & USART_ISR_RXNE)) {
  };

    return HAL_VCP_UART->RDR;

}
