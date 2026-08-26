#include "InGameState.h"
#include "GButton.h"
#include "GLed.h"
#include "GCheat.h"

static bool isActive;

void InGameStateEnter(void)
{
    isActive = true;

    /* TODO: 점수, 타이머, 두더지 상태를 초기화한다. */
    G_LOG(INFO, "InGameState entered. \r\n");
}

void InGameStateUpdate(void)
{
    if (!isActive)
    {
        return;
    }

    SetLedState(LED_ID_1,
                IsButtonPressed(BUTTON_1) ? LED_ON : LED_OFF);
    SetLedState(LED_ID_2,
                IsButtonPressed(BUTTON_2) ? LED_ON : LED_OFF);
    SetLedState(LED_ID_3,
                IsButtonPressed(BUTTON_3) ? LED_ON : LED_OFF);

    /* TODO: 두더지 생성, 점수 및 제한 시간을 갱신한다. */
}

void InGameStateExit(void)
{
    if (!isActive)
    {
        return;
    }

    /* TODO: 활성화된 LED와 인게임 자원을 정리한다. */
    isActive = false;
    G_LOG(INFO, "InGameState exited. \r\n");
}

bool InGameStateIsActive(void)
{
    return isActive;
}
