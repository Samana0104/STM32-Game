#include "GUsart.h"
#include "GCheat.h"
#include "usart.h"
#include "stm32f4xx_hal_uart.h"

#define RX_BUF_SIZE 128
uint8_t rx_data;
uint8_t rx_buf[RX_BUF_SIZE];

void UartInit(void)
{
    if(HAL_UART_GetState(&huart2) != HAL_UART_STATE_READY)
    {
        G_LOG(ERROR, "UART2 is not initialized!\r\n");
    }

    // HAL_UART_Receive_IT(&huart2, &rx_data, 1);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_buf, RX_BUF_SIZE);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

    if (huart->Instance == USART2)
    {
        if (rx_data == 'a')
            printf("Hello STM32 Cortex-M4 USART Polling!\r\n");
        else
            HAL_UART_Transmit(&huart2, rx_buf, RX_BUF_SIZE, 100);

        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_buf, RX_BUF_SIZE);
    }
}
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART2)
    {

    }
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_buf, RX_BUF_SIZE);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    HAL_UART_Receive_DMA(&huart2, rx_buf, RX_BUF_SIZE);
}
