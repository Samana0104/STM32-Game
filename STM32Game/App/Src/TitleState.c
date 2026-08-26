#include "TitleState.h"

#include "GameState.h"
#include "GCheat.h"
#include "GJoystick.h"
#include "Lcd1602.h"

#define WELCOME_DISPLAY_MS 2000U

typedef enum
{
    TITLE_SCREEN_WELCOME = 0,
    TITLE_SCREEN_MENU,
    TITLE_SCREEN_EXITED
} TitleScreen;

static bool isActive;
static TitleScreen currentScreen;
static uint32_t welcomeStartTick;

static void PrintWelcomeScreen(void)
{
    if (Lcd1602IsReady())
    {
        Lcd1602Printf("Welcome to\n\"Whack-a-mole\"");
    }
}

static void PrintTitleMenu(void)
{
    if (Lcd1602IsReady())
    {
        Lcd1602Printf("UP: START\nDOWN: EXIT");
    }
}

static void PrintExitScreen(void)
{
    if (Lcd1602IsReady())
    {
        Lcd1602Printf("GAME EXIT\nUP: START");
    }
}

static void UpdateWelcomeScreen(void)
{
    if ((HAL_GetTick() - welcomeStartTick) < WELCOME_DISPLAY_MS)
    {
        return;
    }

    currentScreen = TITLE_SCREEN_MENU;
    PrintTitleMenu();
}

static void UpdateTitleMenu(void)
{
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
        currentScreen = TITLE_SCREEN_EXITED;
        PrintExitScreen();
        G_LOG(INFO, "Exit selected on title menu. \r\n");
    }
}

static void UpdateExitScreen(void)
{
    if (WasJoystickMoved()
        && GetJoystickDirection() == JOYSTICK_UP)
    {
        GameStateChange(GAME_STATE_PLAYING);
    }
}

void TitleStateEnter(void)
{
    isActive = true;
    currentScreen = TITLE_SCREEN_WELCOME;
    welcomeStartTick = HAL_GetTick();
    PrintWelcomeScreen();
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
        case TITLE_SCREEN_WELCOME:
            UpdateWelcomeScreen();
            break;

        case TITLE_SCREEN_MENU:
            UpdateTitleMenu();
            break;

        case TITLE_SCREEN_EXITED:
            UpdateExitScreen();
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
