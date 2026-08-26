#include "GButton.h"


/*
 * 버튼 정보 구조체
 */
typedef struct _ButtonInfo
{
    GPIO_TypeDef *port;
    uint16_t pin;
    ButtonState state;

} ButtonInfo;


/*
 * 버튼 정보
 *
 * BUTTON_1 -> PA8
 * BUTTON_2 -> PB10
 * BUTTON_3 -> PB4
 * BUTTON_START -> PB5
 */
static ButtonInfo buttons[BUTTON_COUNT] =
{
    { GPIOA, GPIO_PIN_8,  BUTTON_RELEASED },
    { GPIOB, GPIO_PIN_10, BUTTON_RELEASED },
    { GPIOB, GPIO_PIN_4,  BUTTON_RELEASED },
    { GPIOB, GPIO_PIN_5,  BUTTON_RELEASED }
};


/*
 * 버튼 GPIO 초기화
 * 각 버튼은 핀과 GND 사이에 연결하는 Active-Low 방식이다.
 */
void GButtonInit(void)
{
    GPIO_InitTypeDef gpioInit = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    gpioInit.Mode = GPIO_MODE_INPUT;
    gpioInit.Pull = GPIO_PULLUP;

    gpioInit.Pin = GPIO_PIN_8;
    HAL_GPIO_Init(GPIOA, &gpioInit);

    gpioInit.Pin = GPIO_PIN_10 | GPIO_PIN_4 | GPIO_PIN_5;
    HAL_GPIO_Init(GPIOB, &gpioInit);

    UpdateButtonState();
}


/*
 * 버튼 상태 업데이트
 */
void UpdateButtonState(void)
{
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        GPIO_PinState pinState;

        pinState = HAL_GPIO_ReadPin(
            buttons[i].port,
            buttons[i].pin
        );

        /*
         * Pull-Up 방식
         *
         * HIGH(SET)  -> 버튼 안 누름
         * LOW(RESET) -> 버튼 누름
         */
        if (pinState == GPIO_PIN_RESET)
        {
            buttons[i].state = BUTTON_PRESSED;
        }
        else
        {
            buttons[i].state = BUTTON_RELEASED;
        }
    }
}


/*
 * 특정 버튼 상태 반환
 */
ButtonState GetButtonState(ButtonId id)
{
    /*
     * 잘못된 버튼 ID 방지
     */
    if (id < BUTTON_1 || id >= BUTTON_COUNT)
    {
        return BUTTON_RELEASED;
    }

    return buttons[id].state;
}


/*
 * 버튼이 눌려있는지 확인
 */
bool IsButtonPressed(ButtonId id)
{
    /*
     * 잘못된 버튼 ID 방지
     */
    if (id < BUTTON_1 || id >= BUTTON_COUNT)
    {
        return false;
    }

    return GetButtonState(id) == BUTTON_PRESSED;
}


/*
 * 버튼이 떼져있는지 확인
 */
bool IsButtonReleased(ButtonId id)
{
    /*
     * 잘못된 버튼 ID 방지
     */
    if (id < BUTTON_1 || id >= BUTTON_COUNT)
    {
        return false;
    }

    return GetButtonState(id) == BUTTON_RELEASED;
}
