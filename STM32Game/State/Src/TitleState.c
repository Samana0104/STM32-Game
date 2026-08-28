#include "TitleState.h"

#include "GameLcd.h"
#include "GameRecord.h"
#include "GameState.h"
#include "GCheat.h"
#include "GJoystick.h"
#include "ReadyState.h"
#include "SoundPlayer.h"

typedef enum
{
    TITLE_SCREEN_MENU = 0,
    TITLE_SCREEN_RECORD
} TitleScreen;

typedef enum
{
    TITLE_MENU_GAME_START = 0,
    TITLE_MENU_RECORD
} TitleMenuItem;

static bool isActive;
static TitleScreen currentScreen;
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
            currentScreen = TITLE_SCREEN_RECORD;
            GameLcdShowRecord(GameRecordGetBestScore());
            SoundPlayerPlayEffect(SOUND_ID_BUTTON);
        }
    }
}

static void UpdateRecordScreen(void)
{
    if (WasJoystickMoved()
        && GetJoystickDirection() == JOYSTICK_DOWN)
    {
        currentScreen = TITLE_SCREEN_MENU;
        PrintTitleMenu();
        SoundPlayerPlayEffect(SOUND_ID_BUTTON);
    }
}

void TitleStateEnter(void)
{
    isActive = true;
    currentScreen = TITLE_SCREEN_MENU;
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
    SoundPlayerStopBgm();
    G_LOG(INFO, "TitleState exited. \r\n");
}

bool TitleStateIsActive(void)
{
    return isActive;
}
