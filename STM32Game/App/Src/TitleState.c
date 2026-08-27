#include "TitleState.h"

#include "GameLcd.h"
#include "GameRecord.h"
#include "GameState.h"
#include "GCheat.h"
#include "GJoystick.h"

typedef enum
{
    TITLE_SCREEN_MENU = 0,
    TITLE_SCREEN_RECORD
} TitleScreen;

static bool isActive;
static TitleScreen currentScreen;

static void UpdateTitleMenu(void)
{
    if (WasJoystickMoved()
        && GetJoystickDirection() == JOYSTICK_UP)
    {
        GameStateChange(GAME_STATE_PLAYING);
    }
    else if (WasJoystickMoved()
        && GetJoystickDirection() == JOYSTICK_DOWN)
    {
        currentScreen = TITLE_SCREEN_RECORD;
        GameLcdShowRecord(GameRecordGetBestScore(),
                          GameRecordGetLastMissCount());
    }
}

static void UpdateRecordScreen(void)
{
    if (WasJoystickMoved()
        && GetJoystickDirection() == JOYSTICK_UP)
    {
        GameStateChange(GAME_STATE_PLAYING);
    }
    else if (WasJoystickMoved()
        && GetJoystickDirection() == JOYSTICK_DOWN)
    {
        currentScreen = TITLE_SCREEN_MENU;
        GameLcdShowTitleMenu();
    }
}

void TitleStateEnter(void)
{
    isActive = true;
    currentScreen = TITLE_SCREEN_MENU;
    GameLcdShowTitleMenu();
    G_LOG(INFO, "TitleState entered. \r\n");
}

void TitleStateUpdate(void)
{
    if (!isActive)
    {
        return;
    }

    switch (currentScreen)
    {
        case TITLE_SCREEN_MENU:
            UpdateTitleMenu();
            break;

        case TITLE_SCREEN_RECORD:
            UpdateRecordScreen();
            break;

        default:
            break;
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
