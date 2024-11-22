#include "bsp_vcp.h"
#include "usart.h"

//  __attribute__((section(".ram1section")))
// static uint8_t UARTaRxBuffer[10];
//  __attribute__((section(".ram1section"))) 
//  static uint8_t UARTaTxBuffer[10];

USART_TypeDef* HAL_VCP_UART = ((USART_TypeDef *) USART3_BASE);

UART_HandleTypeDef* bsp_uart_handler = &huart3;

void bsp_vcp_uart_init(void) {
    MX_USART3_UART_Init();
}

void bsp_vcp_uart_deinit(void) {
    HAL_UART_DeInit(bsp_uart_handler);
}

void bsp_vcp_tx(char* c){
    // UARTaTxBuffer[0] = (uint8_t)(*c & 0xFFU);
    // memcpy(UARTaTxBuffer, c, 4);
    uint8_t ch = (uint8_t)(*c & 0xFFU);
    
    HAL_UART_Transmit(bsp_uart_handler, &ch, 1, -1);
    // while(!(HAL_VCP_UART->ISR & USART_ISR_TXE_TXFNF)) {
    // };

    // HAL_VCP_UART->TDR = (uint8_t)(*c & 0xFFU); // 16b -> 8b
}




void bsp_vcp_start_it(){
    // if(HAL_UART_Receive_IT(bsp_uart_handler, (uint8_t*)UARTaRxBuffer, 10) != HAL_OK) {
    //     bsp_uart_handler->gState = HAL_UART_STATE_READY;
    // }
}

uint32_t bsp_vcp_rx_it(){
    // uint32_t ch = 0xFFFFFFFF;
    // // __disable_irq();
    // HAL_UART_Abort(bsp_uart_handler);
    // HAL_UART_Receive(bsp_uart_handler, (uint8_t*) &ch, 1, 100);
    // // __enable_irq();
    // return ch;
    
    while(!(HAL_VCP_UART->ISR & USART_ISR_RXNE_RXFNE) ) {
        // return 0xFFFFFFFF;
    };
    return  HAL_VCP_UART->RDR;

}

// #define BSP_VCP_RX_NO_DATA  0xFFFFFFFF  // Define a value to indicate no data is available

// uint32_t bsp_vcp_rx_it() {
//     // Check if the receive register is empty
//     if (HAL_VCP_UART->ISR & USART_ISR_RXNE_RXFNE) {
//         // If data is available, return it
//         return HAL_VCP_UART->RDR;
//     } else {
//         // If no data is available, return the 'no data' indicator
//         // return BSP_VCP_RX_NO_DATA;
//     }
// // }


//     uint32_t ch = HAL_VCP_UART->DR;
 
//      if(HAL_UART_Receive_IT(bsp_uart_handler, (uint8_t*)UARTaRxBuffer, 10) != HAL_OK) {
//         bsp_uart_handler->gState = HAL_UART_STATE_READY;
//     }

//     return ch;
// }