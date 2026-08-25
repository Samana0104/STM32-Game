#include "GButton.h"


/*
 * 버튼별 GPIO Port
 *
 * BUTTON_1 -> PA8
 * BUTTON_2 -> PB10
 * BUTTON_3 -> PB4
 */
static GPIO_TypeDef *buttonPorts[BUTTON_COUNT] =
{
    GPIOA,
    GPIOB,
    GPIOB
};


/*
 * 버튼별 GPIO Pin
 */
static const uint16_t buttonPins[BUTTON_COUNT] =
{
    GPIO_PIN_8,
    GPIO_PIN_10,
    GPIO_PIN_4
};


/*
 * 각 버튼의 현재 상태 저장
 */
static ButtonState buttonStates[BUTTON_COUNT] =
{
    BUTTON_RELEASED,
    BUTTON_RELEASED,
    BUTTON_RELEASED
};


/*
 * 버튼 상태 업데이트
 */
void UpdateButtonState(void)
{
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        GPIO_PinState pinState;

        pinState = HAL_GPIO_ReadPin(buttonPorts[i], buttonPins[i]);

        /*
         * Pull-Up 방식
         *
         * HIGH(SET)   -> 버튼 안 누름
         * LOW(RESET)  -> 버튼 누름
         */
        if (pinState == GPIO_PIN_RESET)
        {
            buttonStates[i] = BUTTON_PRESSED;
        }
        else
        {
            buttonStates[i] = BUTTON_RELEASED;
        }
    }
}


/*
 * 특정 버튼의 현재 상태 반환
 */
ButtonState GetButtonState(ButtonId id)
{
    if (id >= BUTTON_COUNT)
    {
        return BUTTON_RELEASED;
    }

    return buttonStates[id];
}


/*
 * 버튼이 눌려있는지 확인
 */
bool IsButtonPressed(ButtonId id)
{
    return GetButtonState(id) == BUTTON_PRESSED;
}


/*
 * 버튼이 떼져있는지 확인
 */
bool IsButtonReleased(ButtonId id)
{
    return GetButtonState(id) == BUTTON_RELEASED;
}