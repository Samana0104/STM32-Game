#include "RecordState.h"
#include "GameRecord.h"
#include "GameState.h"
#include "GCheat.h"
#include "GJoystick.h"
#include "GLcd1602.h"
#include "SoundPlayer.h"

static bool isActive;
static uint32_t firstRank;

#define LCD_RECORD_MAX 9999U

static uint32_t ClampRecordValue(uint32_t value)
{
    return value > LCD_RECORD_MAX ? LCD_RECORD_MAX : value;
}

static void PrintRecordPage(void)
{
    const uint32_t secondRank = firstRank + 1U;

    Lcd1602Printf("%lu. %lu\n%lu. %lu",
                  (unsigned long)firstRank,
                  (unsigned long)ClampRecordValue(GameRecordGetScore(firstRank)),
                  (unsigned long)secondRank,
                  (unsigned long)ClampRecordValue(GameRecordGetScore(secondRank)));
}

void RecordStateEnter(void)
{
    isActive = true;
    firstRank = 1U;
    SoundPlayerPlayBgm(SOUND_ID_TITLE_BGM);
    PrintRecordPage();
    G_LOG(INFO, "RecordState entered.\r\n");
}

void RecordStateUpdate(void)
{
    if (!isActive || !WasJoystickMoved())
    {
        return;
    }

    const JoystickDirection direction = GetJoystickDirection();

    if (direction == JOYSTICK_RIGHT)
    {
        if (firstRank < (GAME_RECORD_MAX_COUNT - 1U))
        {
            firstRank += 2U;
            PrintRecordPage();
            SoundPlayerPlayEffect(SOUND_ID_BUTTON);
        }
    }
    else if (direction == JOYSTICK_LEFT)
    {
        if (firstRank > 1U)
        {
            firstRank -= 2U;
            PrintRecordPage();
            SoundPlayerPlayEffect(SOUND_ID_BUTTON);
        }
    }
    else if (direction == JOYSTICK_DOWN)
    {
        SoundPlayerPlayEffect(SOUND_ID_BUTTON);
        GameStateChange(GAME_STATE_TITLE);
    }
}

void RecordStateExit(void)
{
    if (!isActive)
    {
        return;
    }

    isActive = false;
    SoundPlayerStopBgm();
    G_LOG(INFO, "RecordState exited.\r\n");
}

bool RecordStateIsActive(void)
{
    return isActive;
}
