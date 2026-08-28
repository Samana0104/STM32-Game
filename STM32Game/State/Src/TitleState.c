#include "TitleState.h"

#include "GameRecord.h"
#include "GameState.h"
#include "GCheat.h"
#include "GJoystick.h"
#include "GLcd1602.h"
#include "ReadyState.h"
#include "SoundPlayer.h"

typedef enum
{
    TITLE_MENU_GAME_START = 0,
    TITLE_MENU_RECORD,
    TITLE_MENU_RECORD_RESET,
    TITLE_MENU_CREDIT,
    TITLE_MENU_COUNT
} TitleMenuItem;

static bool isActive;
static TitleMenuItem selectedMenuItem;

static void PrintTitleMenu(void)
{
    static const char *const labels[TITLE_MENU_COUNT] =
    {
        "1.GameStart",
        "2.Record",
        "3.Reset Record",
        "4.Credit",
    };
    const uint32_t selected = (uint32_t)selectedMenuItem;
    const uint32_t first = (selected / 2U) * 2U;

    Lcd1602Printf("%c%s\n%c%s",
                  selected == first ? '>' : ' ', labels[first],
                  selected == (first + 1U) ? '>' : ' ', labels[first + 1U]);
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
            selectedMenuItem = (TitleMenuItem)(selectedMenuItem - 1);
            PrintTitleMenu();
            SoundPlayerPlayEffect(SOUND_ID_BUTTON);
        }
    }
    else if (direction == JOYSTICK_RIGHT)
    {
        if (selectedMenuItem < (TITLE_MENU_COUNT - 1))
        {
            selectedMenuItem = (TitleMenuItem)(selectedMenuItem + 1);
            PrintTitleMenu();
            SoundPlayerPlayEffect(SOUND_ID_BUTTON);
        }
    }
    else if (direction == JOYSTICK_UP)
    {
        switch (selectedMenuItem)
        {
            case TITLE_MENU_GAME_START:
                ReadyStateSetStage(GAME_STAGE_1);
                GameStateChange(GAME_STATE_READY);
                break;

            case TITLE_MENU_RECORD:
                SoundPlayerPlayEffect(SOUND_ID_BUTTON);
                GameStateChange(GAME_STATE_RECORD);
                break;

            case TITLE_MENU_RECORD_RESET:
                if (GameRecordReset())
                {
                    SoundPlayerPlayEffect(SOUND_ID_SUCCESS);
                    Lcd1602Printf("Record Reset\nComplete!");
                }
                else
                {
                    SoundPlayerPlayEffect(SOUND_ID_FAIL);
                    Lcd1602Printf("Record Reset\nFailed!");
                }
                break;

            case TITLE_MENU_CREDIT:
                SoundPlayerPlayEffect(SOUND_ID_BUTTON);
                GameStateChange(GAME_STATE_CREDIT);
                break;

            case TITLE_MENU_COUNT:
            default:
                break;
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
