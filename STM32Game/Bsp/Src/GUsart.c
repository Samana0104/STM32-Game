#include "GUsart.h"
#include "GCheat.h"
#include "usart.h"
#include <string.h>

#define UART_DMA_RX_SIZE 128U
#define UART_TX_BUFFER_SIZE 512U

static uint8_t dmaRxBuffer[UART_DMA_RX_SIZE];
static char commandBuffer[UART_COMMAND_SIZE];

static volatile bool commandReady;
static bool discardUntilDelimiter;

static uint16_t commandLength;
static uint16_t dmaOldPosition;

static uint8_t txBuffer[UART_TX_BUFFER_SIZE];
static volatile uint16_t txHead;
static volatile uint16_t txTail;
static volatile uint16_t txDmaLength;
static volatile bool txDmaBusy;

static void UartStartTxDma(void)
{
    if (txDmaBusy || (txHead == txTail))
    {
        return;
    }

    if (txHead > txTail)
    {
        txDmaLength = txHead - txTail;
    }
    else
    {
        txDmaLength = UART_TX_BUFFER_SIZE - txTail;
    }

    txDmaBusy = true;

    if (HAL_UART_Transmit_DMA(&huart2, &txBuffer[txTail], txDmaLength) != HAL_OK)
    {
        txDmaBusy = false;
        txDmaLength = 0U;
        return;
    }

    __HAL_DMA_DISABLE_IT(huart2.hdmatx, DMA_IT_HT);
}

void UartInit(void)
{
    if (HAL_UARTEx_ReceiveToIdle_DMA(&huart2, dmaRxBuffer, UART_DMA_RX_SIZE) != HAL_OK)
    {
        G_LOG(DANGER, "UART2 DMA receive start failed.\r\n");
        return;
    }

    commandReady = false;
    discardUntilDelimiter = false;
    commandLength = 0U;
    dmaOldPosition = 0U;
    txHead = 0U;
    txTail = 0U;
    txDmaLength = 0U;
    txDmaBusy = false;

    __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);
}

static void UartStoreReceivedBytes(const uint8_t *data, uint16_t size)
{
    uint8_t byte;
    bool isDelimiter;

    for (uint16_t i = 0; i < size; ++i)
    {
        /* 1. DMA에서 새로 수신된 바이트와 명령 구분 문자를 확인한다. */
        byte = data[i];
        isDelimiter = (byte == '\r') || (byte == '\n');

        /*
         * 2. 완성된 명령을 메인에서 아직 가져가지 않았다면 새 데이터는
         *    저장하지 않는다. 일반 문자를 놓친 경우 현재 줄 전체를
         *    버릴 수 있도록 discard 상태를 기억한다.
         */
        if (commandReady)
        {
            discardUntilDelimiter = !isDelimiter;
            continue;
        }

        /*
         * 3. 명령 앞부분을 이미 놓쳤거나 길이가 초과된 상태라면
         *    다음 줄바꿈까지 남은 바이트를 모두 버린다.
         */
        if (discardUntilDelimiter)
        {
            if (isDelimiter)
            {
                /* 버리던 명령이 끝났으므로 다음 명령부터 다시 받는다. */
                discardUntilDelimiter = false;
            }
            continue;
        }

        /*
         * 4. 줄바꿈을 받으면 현재까지 누적한 문자열을 종료하고
         *    메인 루프에 완성된 명령이 있음을 알린다.
         */
        if (isDelimiter)
        {
            if (commandLength > 0U)
            {
                commandBuffer[commandLength] = '\0';
                commandReady = true;
            }
            continue;
        }

        /* 5. 일반 문자는 명령 버퍼 끝의 null 문자를 남겨두고 누적한다. */
        if (commandLength < (UART_COMMAND_SIZE - 1U))
        {
            commandBuffer[commandLength++] = (char)byte;
        }
        else
        {
            /* 6. 너무 긴 명령은 폐기하고 해당 줄의 끝까지 무시한다. */
            commandLength = 0U;
            discardUntilDelimiter = true;
        }
    }
}

bool UartWriteAsync(const uint8_t *data, uint16_t size)
{
    if ((data == NULL) || (size == 0U) || (size >= UART_TX_BUFFER_SIZE))
    {
        return false;
    }

    const uint32_t primask = __get_PRIMASK();
    __disable_irq();

    const uint16_t used = (uint16_t)((txHead + UART_TX_BUFFER_SIZE - txTail) %
                                     UART_TX_BUFFER_SIZE);
    const uint16_t free = UART_TX_BUFFER_SIZE - used - 1U;

    if (size > free)
    {
        __set_PRIMASK(primask);
        return false;
    }

    for (uint16_t i = 0U; i < size; ++i)
    {
        txBuffer[txHead] = data[i];
        txHead = (uint16_t)((txHead + 1U) % UART_TX_BUFFER_SIZE);
    }

    UartStartTxDma();
    __set_PRIMASK(primask);

    return true;
}

bool UartCommandReady(void)
{
    return commandReady;
}

bool UartReadCommand(char *buffer, uint16_t bufferSize)
{
    if ((buffer == NULL) || (bufferSize == 0U) || !commandReady)
    {
        return false;
    }

    const uint32_t primask = __get_PRIMASK();
    __disable_irq();

    uint16_t copyLength = commandLength;
    if (copyLength >= bufferSize)
    {
        copyLength = bufferSize - 1U;
    }

    memcpy(buffer, commandBuffer, copyLength);
    buffer[copyLength] = '\0';
    commandLength = 0U;
    commandReady = false;

    __set_PRIMASK(primask);

    return true;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != USART2)
    {
        return;
    }

    txTail = (uint16_t)((txTail + txDmaLength) % UART_TX_BUFFER_SIZE);
    txDmaLength = 0U;
    txDmaBusy = false;
    UartStartTxDma();
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t position)
{
    if (huart->Instance != USART2)
    {
        return;
    }

    if (position > dmaOldPosition)
    {
        UartStoreReceivedBytes(&dmaRxBuffer[dmaOldPosition],
                               position - dmaOldPosition);
    }
    else if (position < dmaOldPosition)
    {
        UartStoreReceivedBytes(&dmaRxBuffer[dmaOldPosition],
                               UART_DMA_RX_SIZE - dmaOldPosition);

        if (position > 0U)
        {
            UartStoreReceivedBytes(dmaRxBuffer, position);
        }
    }

    dmaOldPosition = (position == UART_DMA_RX_SIZE) ? 0U : position;
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != USART2)
    {
        return;
    }

    dmaOldPosition = 0U;
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, dmaRxBuffer, UART_DMA_RX_SIZE);

    __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);
}
