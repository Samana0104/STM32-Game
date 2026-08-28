#include "GameMain.h"
#include "GameState.h"
#include "GButton.h"
#include "GCheat.h"
#include "GFnd.h"
#include "GameRecord.h"
#include "GJoystick.h"
#include "GLed.h"
#include "GTimer.h"
#include "GUsart.h"
#include "InGameState.h"
#include "GLcd1602.h"
#include "ReadyState.h"
#include "SoundPlayer.h"
#include "stm32f4xx_hal.h"

#define INITIAL_STAGE 1U

static void GameInit(void)
{
#ifndef NDEBUG
    CheatInit();
#endif

    GledInit();
    GButtonInit();
    GJoystickInit();

    TimerInit();
    SoundPlayerInit();
    UartInit();
    FndInit();
    GameRecordInit();

    SetFndSingleDigit(INITIAL_STAGE);
    SetFnd4DigitNumber(0U);

    Lcd1602Init();

    /* TitleState가 LCD 메뉴 출력과 조이스틱 선택을 담당한다. */
    GameStateInit(GAME_STATE_TITLE);
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
    SoundPlayerUpdate();

    if (GameStateGet() == GAME_STATE_PLAYING && InGameStateIsFinished())
    {
        if (InGameStateGetLife() == 0U)
        {
            GameStateChange(GAME_STATE_RESULT);
        }
        else
        {
            FinishCurrentStage();
        }
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
        Lcd1602Update();
        HAL_Delay(1);
    }

    return 0;
}
