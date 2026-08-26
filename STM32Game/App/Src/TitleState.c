#include "TitleState.h"
#include "GameState.h"
#include "GButton.h"
#include "GCheat.h"

#define START_BUTTON BUTTON_4

static bool isActive;

void TitleStateEnter(void)
{
    isActive = true;
    G_LOG(INFO, "TitleState entered. \r\n");
}

void TitleStateUpdate(void)
{
    if (!isActive)
    {
        return;
    }

    if (WasButtonPressed(START_BUTTON))
    {
        GameStateChange(GAME_STATE_PLAYING);
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
