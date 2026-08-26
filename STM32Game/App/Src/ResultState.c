#include "ResultState.h"

#include "GameState.h"
#include "GCheat.h"
#include "GJoystick.h"
#include "InGameState.h"
#include "Lcd1602.h"

static bool isActive;

void ResultStateEnter(void)
{
    isActive = true;

    if (Lcd1602IsReady())
    {
        Lcd1602Printf("Score:%lu\nUP:RST DN:EXIT",
                      (unsigned long)InGameStateGetScore());
    }

    G_LOG(INFO, "ResultState entered. \r\n");
}

void ResultStateUpdate(void)
{
    if (!isActive || !WasJoystickMoved())
    {
        return;
    }

    if (GetJoystickDirection() == JOYSTICK_UP)
    {
        GameStateChange(GAME_STATE_PLAYING);
    }
    else if (GetJoystickDirection() == JOYSTICK_DOWN)
    {
        GameStateChange(GAME_STATE_TITLE);
    }
}

void ResultStateExit(void)
{
    if (!isActive)
    {
        return;
    }

    isActive = false;
    G_LOG(INFO, "ResultState exited. \r\n");
}

bool ResultStateIsActive(void)
{
    return isActive;
}
