#include "bsp_vcp.h"
#include "usart.h"
USART_TypeDef* HAL_VCP_UART = ((USART_TypeDef *) USART2_BASE);

UART_HandleTypeDef* bsp_uart_handler = &huart2;

void bsp_vcp_uart_init(void) {
    MX_USART2_UART_Init();
}

void bsp_vcp_uart_deinit(void) {
    HAL_UART_DeInit(bsp_uart_handler);
}

void bsp_vcp_tx(char* c){
    while(!(HAL_VCP_UART->SR & USART_SR_TXE)) {
    };
    HAL_VCP_UART->DR = *c;

}

uint32_t bsp_vcp_rx_it(){
    while(!(HAL_VCP_UART->SR & USART_SR_RXNE)) {
    };

    return HAL_VCP_UART->DR;
}
