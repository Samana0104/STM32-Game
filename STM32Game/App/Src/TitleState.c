#include "TitleState.h"
#include "GameState.h"
#include "GCheat.h"
#include "GJoystick.h"
#include "Lcd1602.h"

static bool isActive;

void TitleStateEnter(void)
{
    isActive = true;

    if (Lcd1602IsReady())
    {
        Lcd1602Printf("UP: START\nDOWN: EXIT");
    }

    G_LOG(INFO, "TitleState entered. \r\n");
}

void TitleStateUpdate(void)
{
    if (!isActive)
    {
        return;
    }

    if (!WasJoystickMoved())
    {
        return;
    }

    if (GetJoystickDirection() == JOYSTICK_UP)
    {
        GameStateChange(GAME_STATE_PLAYING);
    }
    else if (GetJoystickDirection() == JOYSTICK_DOWN)
    {
        /* MCU 프로그램은 종료하지 않고 타이틀 메뉴에서 대기한다. */
        G_LOG(INFO, "Exit selected on title menu. \r\n");
    }
}

void TitleStateExit(void)
{
    if (!isActive)
    {
        return;
    }

    isActive = false;
    G_LOG(INFO, "TitleState exited. \r\n");
}

bool TitleStateIsActive(void)
{
    return isActive;
}
