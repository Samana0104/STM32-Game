#ifndef FND1_H
#define FND1_H

#include "main.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>

void FND1_Init(void);
void FND1_DisplayDigit(uint8_t digit, uint8_t showDot);
void FND1_Clear(void);

#endif