#include "bsp_vcp.h"
#include "usart.h"

static uint8_t UARTaRxBuffer[10];

USART_TypeDef* HAL_VCP_UART = ((USART_TypeDef *) UART5_BASE);

UART_HandleTypeDef* bsp_uart_handler = &huart5;

tx_buffer tx_buffer_pool[SIZE_OF_TX_POOL];
uint8_t rx_circular_buffer[SIZE_OF_RX_CIRCULAR_BUFFER];
uint8_t const * rx_tail_ptr;
uint32_t tx_head_position;
uint32_t tx_tail_position;
uint32_t tx_dma;

void bsp_vcp_uart_deinit(void) {
    HAL_UART_MspDeInit(bsp_uart_handler);
}

void bsp_vcp_tx(char* c){
    while(!(HAL_VCP_UART->ISR & USART_ISR_TXE_TXFNF)) {
    };

    HAL_VCP_UART->TDR = (uint8_t)(*c & 0xFFU); // 16b -> 8b
}

void bsp_vcp_start_it(){
    if(HAL_UART_Receive_IT(bsp_uart_handler, (uint8_t*)UARTaRxBuffer, 10) != HAL_OK) {
        bsp_uart_handler->gState = HAL_UART_STATE_READY;
    }
}

void bsp_vcp_uart_init(void) {
    MX_UART5_Init();
    tx_head_position = 0;
    tx_tail_position = 0;
    tx_dma = 0;
    rx_tail_ptr = rx_circular_buffer;
    HAL_UART_Receive_DMA(bsp_uart_handler, rx_circular_buffer,SIZE_OF_RX_CIRCULAR_BUFFER);
}

int bps_vcp_available(){
    uint8_t const * head = rx_circular_buffer + SIZE_OF_RX_CIRCULAR_BUFFER - __HAL_DMA_GET_COUNTER(bsp_uart_handler->hdmarx);
    uint8_t const * tail = rx_tail_ptr;
    if( head>=tail ){        
        return head-tail;
    } else{
        return head-tail+SIZE_OF_RX_CIRCULAR_BUFFER;
    }

}

uint32_t bsp_vcp_rx_it(){
    uint8_t const * head = rx_circular_buffer + SIZE_OF_RX_CIRCULAR_BUFFER - __HAL_DMA_GET_COUNTER(bsp_uart_handler->hdmarx);
    uint8_t const * tail = rx_tail_ptr;
    if(head!=tail)
    {
        char c =  *rx_tail_ptr++;
        if(rx_tail_ptr>=rx_circular_buffer + SIZE_OF_RX_CIRCULAR_BUFFER)
            rx_tail_ptr-=SIZE_OF_RX_CIRCULAR_BUFFER;
        return c;
    }
    else
        return 0;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == bsp_uart_handler && tx_dma == 1)
    {
        if (tx_head_position != tx_tail_position)
        {
            HAL_UART_Transmit_DMA(bsp_uart_handler, tx_buffer_pool[tx_tail_position].data, tx_buffer_pool[tx_tail_position].length);
            tx_tail_position = (tx_tail_position + 1) & (SIZE_OF_TX_POOL-1);
        }
        else
        {
            tx_dma = 0;
        }
    } 
}