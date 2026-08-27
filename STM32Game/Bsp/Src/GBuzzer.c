#include "GBuzzer.h"
#include "GCheat.h"
#include "GTimer.h"

static const TimerOutput timerOutputs[] =
{
    TIMER_OUTPUT_BUZZER_SOUND,
    TIMER_OUTPUT_BUZZER_EFFECT_SOUND,
};

static bool GetTimerOutput(BuzzerOutput output, TimerOutput *timerOutput)
{
    const uint32_t index = (uint32_t)output;

    if ((index >= BUZZER_OUTPUT_MAX_COUNT) || (timerOutput == NULL))
    {
        return false;
    }

    *timerOutput = timerOutputs[index];
    return true;
}

void GBuzzerInit(void)
{
    if (TimerInit() != HAL_OK)
    {
        G_LOG(ERROR, "Buzzer timer initialization failed.\r\n");
    }
}

void PlayFrequency(BuzzerOutput output, uint32_t frequencyHz)
{
    TimerOutput timerOutput;

    if (!GetTimerOutput(output, &timerOutput))
    {
        return;
    }

    if (frequencyHz == 0U)
    {
        StopBuzzer(output);
        return;
    }

    if (TimerSetFrequency(timerOutput, frequencyHz) != HAL_OK)
    {
        G_LOG(ERROR, "Invalid buzzer frequency: %lu Hz.\r\n", (unsigned long)frequencyHz);
        return;
    }

    if (TimerPwmStart(timerOutput) != HAL_OK)
    {
        G_LOG(ERROR, "Buzzer PWM start failed.\r\n");
    }
}

void StopBuzzer(BuzzerOutput output)
{
    TimerOutput timerOutput;

    if (!GetTimerOutput(output, &timerOutput))
    {
        return;
    }

    TimerSetDuty(timerOutput, 0.0f);

    if (TimerPwmStop(timerOutput) != HAL_OK)
    {
        G_LOG(ERROR, "Buzzer PWM stop failed.\r\n");
    }
}
