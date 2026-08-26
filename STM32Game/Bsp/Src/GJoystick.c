#include "GJoystick.h"

#include "adc.h"

/*
 * NUCLEO-F411RE Arduino analog pin mapping
 * VRx -> A5 / PC0 / ADC1_IN10
 * VRy -> A4 / PC1 / ADC1_IN11
 * 조이스틱 전원은 ADC 허용 전압에 맞춰 3.3 V를 사용한다.
 */
#define JOYSTICK_X_ADC_CHANNEL      ADC_CHANNEL_10
#define JOYSTICK_Y_ADC_CHANNEL      ADC_CHANNEL_11

#define JOYSTICK_ADC_MIDPOINT       2048U
#define JOYSTICK_LOW_THRESHOLD      1200U
#define JOYSTICK_HIGH_THRESHOLD     2895U
#define JOYSTICK_ADC_TIMEOUT_MS     2U

static uint16_t joystickX = JOYSTICK_ADC_MIDPOINT;
static uint16_t joystickY = JOYSTICK_ADC_MIDPOINT;
static JoystickDirection joystickDirection = JOYSTICK_CENTER;
static bool joystickMovedEvent;

static bool ReadAdcChannel(uint32_t channel, uint16_t *value)
{
    ADC_ChannelConfTypeDef channelConfig = {0};

    if (value == NULL)
    {
        return false;
    }

    channelConfig.Channel = channel;
    channelConfig.Rank = 1U;
    channelConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;

    if (HAL_ADC_ConfigChannel(&hadc1, &channelConfig) != HAL_OK
        || HAL_ADC_Start(&hadc1) != HAL_OK)
    {
        return false;
    }

    if (HAL_ADC_PollForConversion(&hadc1, JOYSTICK_ADC_TIMEOUT_MS) != HAL_OK)
    {
        HAL_ADC_Stop(&hadc1);
        return false;
    }

    *value = (uint16_t)HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return true;
}

static uint16_t GetDistanceFromCenter(uint16_t value)
{
    if (value >= JOYSTICK_ADC_MIDPOINT)
    {
        return value - JOYSTICK_ADC_MIDPOINT;
    }

    return JOYSTICK_ADC_MIDPOINT - value;
}

static JoystickDirection CalculateDirection(uint16_t x, uint16_t y)
{
    bool xMoved = (x < JOYSTICK_LOW_THRESHOLD)
        || (x > JOYSTICK_HIGH_THRESHOLD);
    bool yMoved = (y < JOYSTICK_LOW_THRESHOLD)
        || (y > JOYSTICK_HIGH_THRESHOLD);

    if (!xMoved && !yMoved)
    {
        return JOYSTICK_CENTER;
    }

    /* 대각선 입력은 중앙에서 더 멀리 움직인 축을 대표 방향으로 삼는다. */
    if (xMoved
        && (!yMoved
            || GetDistanceFromCenter(x) >= GetDistanceFromCenter(y)))
    {
        return x < JOYSTICK_ADC_MIDPOINT
            ? JOYSTICK_LEFT
            : JOYSTICK_RIGHT;
    }

    return y < JOYSTICK_ADC_MIDPOINT
        ? JOYSTICK_DOWN
        : JOYSTICK_UP;
}

void GJoystickInit(void)
{
    uint16_t initialX;
    uint16_t initialY;

    joystickX = JOYSTICK_ADC_MIDPOINT;
    joystickY = JOYSTICK_ADC_MIDPOINT;
    joystickDirection = JOYSTICK_CENTER;
    joystickMovedEvent = false;

    /* 부팅할 때 기울어져 있어도 시작 이벤트가 즉시 발생하지 않게 한다. */
    if (ReadAdcChannel(JOYSTICK_X_ADC_CHANNEL, &initialX)
        && ReadAdcChannel(JOYSTICK_Y_ADC_CHANNEL, &initialY))
    {
        joystickX = initialX;
        joystickY = initialY;
        joystickDirection = CalculateDirection(initialX, initialY);
    }
}

void UpdateJoystickState(void)
{
    uint16_t nextX;
    uint16_t nextY;
    JoystickDirection nextDirection;

    joystickMovedEvent = false;

    if (!ReadAdcChannel(JOYSTICK_X_ADC_CHANNEL, &nextX)
        || !ReadAdcChannel(JOYSTICK_Y_ADC_CHANNEL, &nextY))
    {
        return;
    }

    nextDirection = CalculateDirection(nextX, nextY);
    joystickMovedEvent = (joystickDirection == JOYSTICK_CENTER)
        && (nextDirection != JOYSTICK_CENTER);

    joystickX = nextX;
    joystickY = nextY;
    joystickDirection = nextDirection;
}

JoystickDirection GetJoystickDirection(void)
{
    return joystickDirection;
}

bool IsJoystickMoved(void)
{
    return joystickDirection != JOYSTICK_CENTER;
}

bool WasJoystickMoved(void)
{
    return joystickMovedEvent;
}

uint16_t GetJoystickX(void)
{
    return joystickX;
}

uint16_t GetJoystickY(void)
{
    return joystickY;
}
