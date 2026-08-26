#include "GameMain.h"
#include "GameState.h"
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
    GameStateInit(GAME_STATE_PLAYING);

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

    while (1)
    {
        GameUpdate();
    }

    G_LOG(INFO, "GameMain exited. \r\n");
    return 0;
}
