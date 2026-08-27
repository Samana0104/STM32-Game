#pragma once
#include "main.h"
#include "stm32f4xx_hal.h"

#define FND_4DIGIT_COUNT 4
#define FND_TOTAL_DIGITS 5
#define FND_MAX_4DIGIT   9999

typedef struct _FndPin
{
    GPIO_TypeDef *port;
    uint16_t pin;
} FndPin;

void FndInit(void);
void SetFndSingleDigit(uint8_t digit);
void SetFnd4DigitNumber(uint16_t number);
void FndUpdate(void);
void FndClear(void);
