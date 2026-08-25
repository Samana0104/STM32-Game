#pragma once
#include "main.h"
#include <stdbool.h>
#include <stdint.h>

#define UART_COMMAND_SIZE 128U

void UartInit(void);
bool UartCommandReady(void);
bool UartReadCommand(char *buffer, uint16_t bufferSize);
bool UartWriteAsync(const uint8_t *data, uint16_t size);
