#include "InGameState.h"

static bool isActive;
static uint32_t startTick;

void InGameStateEnter(void)
{
    startTick = HAL_GetTick();
    isActive = true;

    /* TODO: 점수, 타이머, 두더지 상태를 초기화한다. */
}

void InGameStateUpdate(void)
{
    if (!isActive)
    {
        return;
    }

    /* TODO: 두더지 생성, 버튼 판정, 점수 및 제한 시간을 갱신한다. */
}

void InGameStateExit(void)
{
    if (!isActive)
    {
        return;
    }

    /* TODO: 활성화된 LED와 인게임 자원을 정리한다. */
    isActive = false;
}

bool InGameStateIsActive(void)
{
    return isActive;
}

uint32_t InGameStateGetElapsedMs(void)
{
    if (!isActive)
    {
        return 0U;
    }

    return HAL_GetTick() - startTick;
}
