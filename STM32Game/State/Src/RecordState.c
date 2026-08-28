#include "RecordState.h"
#include "GameLcd.h"
#include "GameRecord.h"
#include "GameState.h"
#include "GCheat.h"
#include "GJoystick.h"
#include "SoundPlayer.h"

static bool isActive;
static uint32_t firstRank;

static void PrintRecordPage(void)
{
    const uint32_t secondRank = firstRank + 1U;

    GameLcdShowRecordPage(firstRank,
                          GameRecordGetScore(firstRank),
                          secondRank,
                          GameRecordGetScore(secondRank));
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
