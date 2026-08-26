#include "GJoystick.h"

#include "stm32f4xx_hal.h"

/*
 * NUCLEO-F411RE Arduino analog pin mapping
 * VRx -> A5 / PC0 / ADC1_IN10
 * VRy -> A4 / PC1 / ADC1_IN11
 * 조이스틱 전원은 ADC 허용 전압에 맞춰 3.3 V를 사용한다.
 */
#define JOYSTICK_GPIO_PORT          GPIOC
#define JOYSTICK_X_PIN              GPIO_PIN_0
#define JOYSTICK_Y_PIN              GPIO_PIN_1
#define JOYSTICK_X_ADC_CHANNEL      10U
#define JOYSTICK_Y_ADC_CHANNEL      11U

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
    uint32_t startTick;

    if (value == NULL)
    {
        return false;
    }

    ADC1->SQR3 = (ADC1->SQR3 & ~ADC_SQR3_SQ1)
        | (channel << ADC_SQR3_SQ1_Pos);
    ADC1->SR = 0U;
    ADC1->CR2 |= ADC_CR2_SWSTART;

    startTick = HAL_GetTick();
    while ((ADC1->SR & ADC_SR_EOC) == 0U)
    {
        if ((HAL_GetTick() - startTick) >= JOYSTICK_ADC_TIMEOUT_MS)
        {
            return false;
        }
    }

    *value = (uint16_t)ADC1->DR;
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
    GPIO_InitTypeDef gpioInit = {0};
    uint16_t initialX;
    uint16_t initialY;

    __HAL_RCC_GPIOC_CLK_ENABLE();
    gpioInit.Pin = JOYSTICK_X_PIN | JOYSTICK_Y_PIN;
    gpioInit.Mode = GPIO_MODE_ANALOG;
    gpioInit.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(JOYSTICK_GPIO_PORT, &gpioInit);

    __HAL_RCC_ADC1_CLK_ENABLE();

    /* APB2 84 MHz / 4 = ADC clock 21 MHz, 12-bit single conversion. */
    ADC->CCR = (ADC->CCR & ~ADC_CCR_ADCPRE) | ADC_CCR_ADCPRE_0;
    ADC1->CR1 = 0U;
    ADC1->CR2 = 0U;
    ADC1->SQR1 = 0U;
    ADC1->SQR2 = 0U;
    ADC1->SQR3 = 0U;
    ADC1->SMPR1 = (ADC1->SMPR1
        & ~(ADC_SMPR1_SMP10 | ADC_SMPR1_SMP11))
        | ADC_SMPR1_SMP10_2
        | ADC_SMPR1_SMP11_2;
    ADC1->CR2 = ADC_CR2_ADON;

    /* ADC 전원 안정화 시간을 확보한다. */
    for (volatile uint32_t i = 0U; i < 100U; i++)
    {
        __NOP();
    }

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
