#include "TitleState.h"

#include "GameLcd.h"
#include "GameState.h"
#include "GCheat.h"
#include "GJoystick.h"
#include "ReadyState.h"
#include "SoundPlayer.h"

typedef enum
{
    TITLE_MENU_GAME_START = 0,
    TITLE_MENU_RECORD
} TitleMenuItem;

static bool isActive;
static TitleMenuItem selectedMenuItem;

static void PrintTitleMenu(void)
{
    GameLcdShowTitleMenu(selectedMenuItem == TITLE_MENU_RECORD);
}

static void UpdateTitleMenu(void)
{
    JoystickDirection direction;

    if (!WasJoystickMoved())
    {
        return;
    }

    direction = GetJoystickDirection();

    if (direction == JOYSTICK_LEFT)
    {
        if (selectedMenuItem != TITLE_MENU_GAME_START)
        {
            selectedMenuItem = TITLE_MENU_GAME_START;
            PrintTitleMenu();
            SoundPlayerPlayEffect(SOUND_ID_BUTTON);
        }
    }
    else if (direction == JOYSTICK_RIGHT)
    {
        if (selectedMenuItem != TITLE_MENU_RECORD)
        {
            selectedMenuItem = TITLE_MENU_RECORD;
            PrintTitleMenu();
            SoundPlayerPlayEffect(SOUND_ID_BUTTON);
        }
    }
    else if (direction == JOYSTICK_UP)
    {
        if (selectedMenuItem == TITLE_MENU_GAME_START)
        {
            ReadyStateSetStage(GAME_STAGE_1);
            GameStateChange(GAME_STATE_READY);
        }
        else
        {
            SoundPlayerPlayEffect(SOUND_ID_BUTTON);
            GameStateChange(GAME_STATE_RECORD);
        }
    }
}

void TitleStateEnter(void)
{
    isActive = true;
    selectedMenuItem = TITLE_MENU_GAME_START;
    SoundPlayerPlayBgm(SOUND_ID_TITLE_BGM);
    PrintTitleMenu();
    G_LOG(INFO, "TitleState entered. \r\n");
}

void TitleStateUpdate(void)
{
    if (!isActive)
    {
        return;
    }

    UpdateTitleMenu();
}

void TitleStateExit(void)
{
    if (!isActive)
    {
        return;
    }

    isActive = false;
    SoundPlayerStopBgm();
    G_LOG(INFO, "TitleState exited. \r\n");
}

bool TitleStateIsActive(void)
{
    return isActive;
}
