#include "GameMain.h"
#include "GameState.h"
#include "GCheat.h"
#include "GLed.h"
#include "GButton.h"
#include "GUsart.h"
#include "GFnd.h"
#include "stm32f4xx_hal.h"

static void GameInit(void)
{
    GledInit();
    UartInit();
    GButtonInit();
    FndInit();
    GameStateInit(GAME_STATE_TITLE);

#ifndef NDEBUG
    CheatInit();
#endif
}

static void GameUpdate(void)
{
    UpdateButtonState();
    GameStateUpdate();

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

    // SetFndSingleDigit(3);     /* 1자리 FND에 '3' 표시 */
    // SetFnd4DigitNumber(2026);  /* 4자리 FND에 '2026' 표시 */

    while (1)
    {
        GameUpdate();

        // UpdateFnd(); /* 5개 디스플레이를 순환 점등 */
        // HAL_Delay(1);

        /* 3. 루프 지연 (디바운싱 및 CPU 점유율 조절) */
        HAL_Delay(10);
    }

    G_LOG(INFO, "GameMain exited. \r\n");
    return 0;
}
