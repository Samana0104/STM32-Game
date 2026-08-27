#pragma once

#include "main.h"

/* The timer/channel pairing stays private to prevent invalid combinations. */
typedef enum
{
    TIMER_OUTPUT_BUZZER_SOUND = 0,
    TIMER_OUTPUT_BUZZER_EFFECT_SOUND,
    TIMER_OUTPUT_MAX_COUNT
} TimerOutput;

void TimerInit(void);
HAL_StatusTypeDef TimerPwmStart(TimerOutput output);
HAL_StatusTypeDef TimerPwmStop(TimerOutput output);

void TimerSetDuty(TimerOutput output, float dutyPercent);
HAL_StatusTypeDef TimerSetFrequency(TimerOutput output, uint32_t frequencyHz);
