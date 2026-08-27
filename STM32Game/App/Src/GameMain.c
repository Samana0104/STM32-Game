#include "GameMain.h"
#include "GameState.h"
#include "GButton.h"
#include "GCheat.h"
#include "GFnd.h"
#include "GameRecord.h"
#include "GJoystick.h"
#include "GLed.h"
#include "GUsart.h"
#include "InGameState.h"
#include "GLcd1602.h"
#include "ReadyState.h"

#include "stm32f4xx_hal.h"

#define INITIAL_STAGE 1U

static void GameInit(void)
{
    GledInit();
    GButtonInit();
    GJoystickInit();
    UartInit();
    FndInit();
    GameRecordInit();

    SetFndSingleDigit(INITIAL_STAGE);
    SetFnd4DigitNumber(0U);

    if (!Lcd1602Init())
    {
        G_LOG(ERROR, "LCD initialization failed.\r\n");
    }

    /* TitleState가 LCD 메뉴 출력과 조이스틱 선택을 담당한다. */
    GameStateInit(GAME_STATE_TITLE);

#ifndef NDEBUG
    CheatInit();
#endif
}

static void FinishCurrentStage(void)
{
    GameStage currentStage = ReadyStateGetStage();

    if (currentStage < GAME_STAGE_5)
    {
        ReadyStateSetStage((GameStage)(currentStage + 1));
        GameStateChange(GAME_STATE_READY);
    }
    else
    {
        GameStateChange(GAME_STATE_RESULT);
    }
}

static void GameUpdate(void)
{
    UpdateButtonState();
    UpdateJoystickState();

    GameStateUpdate();

    if (GameStateGet() == GAME_STATE_PLAYING
        && InGameStateIsFinished())
    {
        FinishCurrentStage();
    }

#ifndef NDEBUG
    CheatUpdate();
    CheatFrameTick();
#endif
}

int GameMain(void)
{
    GameInit();
    G_LOG(INFO, "Game started successfully.\r\n");

    while (1)
    {
        GameUpdate();
        FndUpdate();
        HAL_Delay(1);
    }

    return 0;
}
