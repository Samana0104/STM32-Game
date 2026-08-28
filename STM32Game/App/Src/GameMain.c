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
#include "GLcd1602.h"
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

static void GameUpdate(void)
{
    UpdateButtonState();
    UpdateJoystickState();
    FndUpdate();
    Lcd1602Update();
    SoundPlayerUpdate();

    GameStateUpdate();

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
        HAL_Delay(1);
    }

    return 0;
}
