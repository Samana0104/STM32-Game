#include "GTimer.h"
#include "tim.h"

typedef struct
{
    TIM_HandleTypeDef *htim;
    uint32_t channel;
} TimerOutputConfig;

static const TimerOutputConfig timerOutputConfigs[] = 
{
    {&htim2, TIM_CHANNEL_1},
    {&htim5, TIM_CHANNEL_2},
};

static const TimerOutputConfig *TimerGetConfig(TimerOutput output)
{
    const uint32_t index = (uint32_t)output;

    /* A negative enum value converts to a large unsigned value and fails here. */
    if (index >= TIMER_OUTPUT_MAX_COUNT)
    {
        return NULL;
    }

    const TimerOutputConfig *config = &timerOutputConfigs[index];

    if ((config->htim == NULL) || (config->htim->Instance == NULL) || !IS_TIM_CCX_INSTANCE(config->htim->Instance, config->channel))
    {
        return NULL;
    }

    return config;
}

static uint32_t TimerGetInputClock(const TIM_TypeDef *instance)
{
    RCC_ClkInitTypeDef clocks;
    uint32_t flashLatency;

    HAL_RCC_GetClockConfig(&clocks, &flashLatency);

    if ((instance == TIM1) || (instance == TIM9) ||
        (instance == TIM10) || (instance == TIM11))
    {
        uint32_t clock = HAL_RCC_GetPCLK2Freq();
        return (clocks.APB2CLKDivider == RCC_HCLK_DIV1) ? clock : clock * 2U;
    }

    {
        uint32_t clock = HAL_RCC_GetPCLK1Freq();
        return (clocks.APB1CLKDivider == RCC_HCLK_DIV1) ? clock : clock * 2U;
    }
}

HAL_StatusTypeDef TimerInit(void)
{
    TimerSetDuty(TIMER_OUTPUT_BUZZER_SOUND, 0.0f);
    TimerSetDuty(TIMER_OUTPUT_BUZZER_EFFECT_SOUND, 0.0f);
    return HAL_OK;
}

HAL_StatusTypeDef TimerPwmStart(TimerOutput output)
{
    const TimerOutputConfig *config = TimerGetConfig(output);

    if (config == NULL)
    {
        return HAL_ERROR;
    }

    if (TIM_CHANNEL_STATE_GET(config->htim, config->channel) ==
        HAL_TIM_CHANNEL_STATE_BUSY)
    {
        return HAL_OK;
    }

    return HAL_TIM_PWM_Start(config->htim, config->channel);
}

HAL_StatusTypeDef TimerPwmStop(TimerOutput output)
{
    const TimerOutputConfig *config = TimerGetConfig(output);

    if (config == NULL)
    {
        return HAL_ERROR;
    }

    if (TIM_CHANNEL_STATE_GET(config->htim, config->channel) ==
        HAL_TIM_CHANNEL_STATE_READY)
    {
        return HAL_OK;
    }

    return HAL_TIM_PWM_Stop(config->htim, config->channel);
}

void TimerSetDuty(TimerOutput output, float dutyPercent)
{
    const TimerOutputConfig *config = TimerGetConfig(output);

    if (config == NULL)
    {
        return;
    }

    if (dutyPercent <= 0.0f)
    {
        dutyPercent = 0.0f;
    }
    else if (dutyPercent > 100.0f)
    {
        dutyPercent = 100.0f;
    }

    const uint32_t period = __HAL_TIM_GET_AUTORELOAD(config->htim) + 1U;
    const uint32_t pulse = (uint32_t)((period * dutyPercent) * 0.01f);

    __HAL_TIM_SET_COMPARE(config->htim, config->channel, pulse);
}

HAL_StatusTypeDef TimerSetFrequency(TimerOutput output, uint32_t frequencyHz)
{
    const TimerOutputConfig *config = TimerGetConfig(output);

    if ((config == NULL) || (frequencyHz == 0U))
    {
        return HAL_ERROR;
    }

    const uint32_t timerClock = TimerGetInputClock(config->htim->Instance);
    const uint32_t counterClock = timerClock / (config->htim->Instance->PSC + 1U);
    const uint64_t period = (uint64_t)counterClock / frequencyHz;

    if ((period == 0U) || (period > 0x100000000ULL))
    {
        return HAL_ERROR;
    }

    __HAL_TIM_SET_AUTORELOAD(config->htim, (uint32_t)period - 1U);
    __HAL_TIM_SET_COMPARE(config->htim, config->channel, (uint32_t)period / 2U);
    __HAL_TIM_SET_COUNTER(config->htim, 0U);

    return HAL_OK;
}
