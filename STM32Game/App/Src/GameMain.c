#include "GameMain.h"
#include "GameState.h"
#include "GButton.h"
#include "GCheat.h"
#include "GFnd.h"
#include "GJoystick.h"
#include "GLed.h"
#include "GUsart.h"
#include "InGameState.h"
#include "Lcd1602.h"

#include "stm32f4xx_hal.h"

#define INITIAL_STAGE 1U

static void GameInit(void)
{
    GledInit();
    GButtonInit();
    GJoystickInit();
    UartInit();
    FndInit();

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

static void UpdateResultMenu(void)
{
    if (GameStateGet() != GAME_STATE_RESULT || !WasJoystickMoved())
    {
        return;
    }

    if (GetJoystickDirection() == JOYSTICK_UP)
    {
        GameStateChange(GAME_STATE_PLAYING);
        G_LOG(INFO, "Game restarted by joystick UP.\r\n");
    }
    else if (GetJoystickDirection() == JOYSTICK_DOWN)
    {
        /* EXIT은 펌웨어 종료가 아니라 타이틀 메뉴 복귀로 처리한다. */
        GameStateChange(GAME_STATE_TITLE);
    }
}

static void GameUpdate(void)
{
    UpdateButtonState();
    UpdateJoystickState();

    UpdateResultMenu();
    GameStateUpdate();

    if (GameStateGet() == GAME_STATE_PLAYING
        && InGameStateIsFinished())
    {
        GameStateChange(GAME_STATE_RESULT);

        if (Lcd1602IsReady())
        {
            /* 점수는 4자리 FND에 유지하고 LCD는 메뉴만 표시한다. */
            Lcd1602Printf("UP: RESTART\nDOWN: EXIT");
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
        UpdateFnd();
        HAL_Delay(1);
    }

    return 0;
}
