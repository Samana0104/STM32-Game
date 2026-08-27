#pragma once

#include "main.h"

typedef enum
{
    BUZZER_OUTPUT_SOUND = 0,
    BUZZER_OUTPUT_EFFECT_SOUND,
    BUZZER_OUTPUT_MAX_COUNT
} BuzzerOutput;

void GBuzzerInit(void);
void PlayFrequency(BuzzerOutput output, uint32_t frequencyHz);
void StopBuzzer(BuzzerOutput output);
