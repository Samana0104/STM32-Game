#include "GButton.h"


#define BUTTON_DEBOUNCE_MS 20U


/*
 * 버튼 정보 구조체
 */
typedef struct _ButtonInfo
{
    GPIO_TypeDef *port;
    uint16_t pin;
    ButtonState state;
    ButtonState rawState;
    uint32_t rawStateChangedTick;
    bool pressedEvent;
    bool releasedEvent;

} ButtonInfo;


/*
 * 버튼 정보
 *
 * NUCLEO-F411RE Arduino D8부터 D2까지 순서대로 사용한다.
 *
 * BUTTON_1 -> PA9  (D8)
 * BUTTON_2 -> PA8  (D7)
 * BUTTON_3 -> PB10 (D6)
 * BUTTON_4 -> PB4  (D5)
 * BUTTON_5 -> PB5  (D4)
 * BUTTON_6 -> PB3  (D3, SWO 기능 미사용)
 * BUTTON_7 -> PA10 (D2)
 */
static ButtonInfo buttons[BUTTON_COUNT] =
{
    { GPIOA, GPIO_PIN_9,  BUTTON_RELEASED, BUTTON_RELEASED, 0U, false, false },
    { GPIOA, GPIO_PIN_8,  BUTTON_RELEASED, BUTTON_RELEASED, 0U, false, false },
    { GPIOB, GPIO_PIN_10, BUTTON_RELEASED, BUTTON_RELEASED, 0U, false, false },
    { GPIOB, GPIO_PIN_4,  BUTTON_RELEASED, BUTTON_RELEASED, 0U, false, false },
    { GPIOB, GPIO_PIN_5,  BUTTON_RELEASED, BUTTON_RELEASED, 0U, false, false },
    { GPIOB, GPIO_PIN_3,  BUTTON_RELEASED, BUTTON_RELEASED, 0U, false, false },
    { GPIOA, GPIO_PIN_10, BUTTON_RELEASED, BUTTON_RELEASED, 0U, false, false }
};


static bool IsButtonIdValid(ButtonId id)
{
    return id >= BUTTON_1 && id < BUTTON_COUNT;
}


static ButtonState ReadButtonState(const ButtonInfo *button)
{
    GPIO_PinState pinState = HAL_GPIO_ReadPin(button->port, button->pin);

    /* Pull-up 방식이므로 LOW일 때 눌림 상태이다. */
    return pinState == GPIO_PIN_RESET
        ? BUTTON_PRESSED
        : BUTTON_RELEASED;
}


/*
 * 버튼 GPIO 초기화
 * 각 버튼은 핀과 GND 사이에 연결하는 Active-Low 방식이다.
 */
void GButtonInit(void)
{
    GPIO_InitTypeDef gpioInit = {0};
    uint32_t currentTick;

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    gpioInit.Mode = GPIO_MODE_INPUT;
    gpioInit.Pull = GPIO_PULLUP;

    gpioInit.Pin = GPIO_PIN_9 | GPIO_PIN_8 | GPIO_PIN_10;
    HAL_GPIO_Init(GPIOA, &gpioInit);

    gpioInit.Pin = GPIO_PIN_10 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_3;
    HAL_GPIO_Init(GPIOB, &gpioInit);

    currentTick = HAL_GetTick();
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        ButtonState initialState = ReadButtonState(&buttons[i]);

        buttons[i].state = initialState;
        buttons[i].rawState = initialState;
        buttons[i].rawStateChangedTick = currentTick;
        buttons[i].pressedEvent = false;
        buttons[i].releasedEvent = false;
    }
}


/*
 * 버튼 상태 업데이트
 */
void UpdateButtonState(void)
{
    uint32_t currentTick = HAL_GetTick();

    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        ButtonState sampledState = ReadButtonState(&buttons[i]);

        /* 이벤트는 UpdateButtonState() 호출 한 번 동안만 유지한다. */
        buttons[i].pressedEvent = false;
        buttons[i].releasedEvent = false;

        if (sampledState != buttons[i].rawState)
        {
            buttons[i].rawState = sampledState;
            buttons[i].rawStateChangedTick = currentTick;
        }
        else if (sampledState != buttons[i].state
            && (currentTick - buttons[i].rawStateChangedTick)
                >= BUTTON_DEBOUNCE_MS)
        {
            buttons[i].state = sampledState;
            buttons[i].pressedEvent = sampledState == BUTTON_PRESSED;
            buttons[i].releasedEvent = sampledState == BUTTON_RELEASED;
        }
    }
}


/*
 * 특정 버튼 상태 반환
 */
ButtonState GetButtonState(ButtonId id)
{
    if (!IsButtonIdValid(id))
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
    if (!IsButtonIdValid(id))
    {
        return false;
    }

    return GetButtonState(id) == BUTTON_PRESSED;
}


/*
 * 디바운싱이 끝난 뒤 버튼이 눌린 첫 업데이트에서만 true를 반환한다.
 */
bool WasButtonPressed(ButtonId id)
{
    if (!IsButtonIdValid(id))
    {
        return false;
    }

    return buttons[id].pressedEvent;
}


/*
 * 버튼이 떼져있는지 확인
 */
bool IsButtonReleased(ButtonId id)
{
    if (!IsButtonIdValid(id))
    {
        return false;
    }

    return GetButtonState(id) == BUTTON_RELEASED;
}

<<<<<<< Updated upstream

/*
 * 디바운싱이 끝난 뒤 버튼이 떼어진 첫 업데이트에서만 true를 반환한다.
 */
bool WasButtonReleased(ButtonId id)
{
    if (!IsButtonIdValid(id))
=======
/*
 * 특정 버튼이 방금 막 눌렸는지 확인 (단발성 인식)
 * 꾹 누르고 있어도 최초 1회만 true를 반환하고, 뗄 때까지 false를 유지함
 */
bool IsButtonClicked(ButtonId id)
{
    if (id < BUTTON_1 || id >= BUTTON_COUNT)
>>>>>>> Stashed changes
    {
        return false;
    }

<<<<<<< Updated upstream
    return buttons[id].releasedEvent;
}
=======
    // 직전 프레임의 버튼 상태를 기억하기 위한 static 배열
    static ButtonState lastStates[BUTTON_COUNT] = { BUTTON_RELEASED, };
    
    ButtonState currentState = GetButtonState(id);
    bool isClicked = false;

    // 이전에 떼어져 있었고(RELEASED), 이번에 눌렸다면(PRESSED) -> 딱 1회 참 인정
    if (lastStates[id] == BUTTON_RELEASED && currentState == BUTTON_PRESSED)
    {
        isClicked = true;
    }

    // 현재 상태를 다음 비교를 위해 저장
    lastStates[id] = currentState;

    return isClicked;
}
>>>>>>> Stashed changes
