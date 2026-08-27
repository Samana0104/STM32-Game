#include "GBuzzer.h"
#include "GCheat.h"
#include "GTimer.h"

void GBuzzerInit(void)
{
    if (TimerInit() != HAL_OK)
    {
        G_LOG(ERROR, "Buzzer timer initialization failed.\r\n");
    }
}

void PlayFrequency(uint32_t frequencyHz)
{
    if (frequencyHz == 0U)
    {
        StopBuzzer();
        return;
    }

    if (TimerSetFrequency(TIMER_OUTPUT_BUZZER, frequencyHz) != HAL_OK)
    {
        G_LOG(ERROR, "Invalid buzzer frequency: %lu Hz.\r\n", (unsigned long)frequencyHz);
        return;
    }

    if (TimerPwmStart(TIMER_OUTPUT_BUZZER) != HAL_OK)
    {
        G_LOG(ERROR, "Buzzer PWM start failed.\r\n");
    }
}

void StopBuzzer(void)
{
    TimerSetDuty(TIMER_OUTPUT_BUZZER, 0.0f);

    if (TimerPwmStop(TIMER_OUTPUT_BUZZER) != HAL_OK)
    {
        G_LOG(ERROR, "Buzzer PWM stop failed.\r\n");
    }
}
