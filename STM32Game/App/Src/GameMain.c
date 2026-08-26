#include "GameMain.h"
#include "GCheat.h"
#include "GLed.h"
#include "GButton.h"
#include "GUsart.h"
#include "stm32f4xx_hal.h"

static void GameInit(void)
{
    GledInit();
    UartInit();
    GButtonInit();

#ifndef NDEBUG
    CheatInit();
#endif

}

static void GameUpdate(void)
{
#ifndef NDEBUG
    CheatUpdate();
    /* 한 프레임의 모든 작업이 끝난 시점에서 프레임 시간을 측정한다. */
    CheatFrameTick();
#endif
}

int GameMain(void)
{
    GameInit();
    G_LOG(INFO, "Game started successfully. \r\n");

    while (1)
    {
        /* 1. 데이터 업데이트 */
        GameUpdate();
        UpdateButtonState(); // 현재 핀 상태를 한 번만 읽어옵니다.

        /* 2. 각 버튼 독립 제어 */
        
        // BUTTON_1 제어
        if (IsButtonPressed(BUTTON_1)) {
            SetLedState(LED_ID_1, LED_ON);
        } else {
            SetLedState(LED_ID_1, LED_OFF);
        }

        // BUTTON_2 제어
        if (IsButtonPressed(BUTTON_2)) {
            SetLedState(LED_ID_2, LED_ON);
        } else {
            SetLedState(LED_ID_2, LED_OFF);
        }

        // BUTTON_3 제어
        if (IsButtonPressed(BUTTON_3)) {
            SetLedState(LED_ID_3, LED_ON);
        } else {
            SetLedState(LED_ID_3, LED_OFF);
        }

        /* 3. 루프 지연 (디바운싱 및 CPU 점유율 조절) */
        HAL_Delay(10);

    }

    G_LOG(INFO, "GameMain exited. \r\n");
    return 0;
}
