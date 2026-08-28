#include "ResultState.h"

#include "GameLcd.h"
#include "GameRecord.h"
#include "GameState.h"
#include "GCheat.h"
#include "GJoystick.h"
#include "InGameState.h"
#include "ReadyState.h"
#include "SoundPlayer.h"

#define GAME_OVER_DISPLAY_MS 2000U

static bool isActive;
static bool isRecordVisible;
static uint32_t gameOverStartTick;

void ResultStateEnter(void)
{
    isActive = true;
    isRecordVisible = false;
    gameOverStartTick = HAL_GetTick();

    GameRecordSave(InGameStateGetScore());
    GameLcdShowGameOver();

    if (InGameStateGetLife() == 0U)
    {
        SoundPlayerPlayEffect(SOUND_ID_FAIL);
    }
    else
    {
        SoundPlayerPlayBgm(SOUND_ID_CANON);
    }

    G_LOG(INFO, "ResultState entered. \r\n");
}

void ResultStateUpdate(void)
{
    if (!isActive)
    {
        return;
    }

    if (!isRecordVisible)
    {
        if ((HAL_GetTick() - gameOverStartTick) < GAME_OVER_DISPLAY_MS)
        {
            return;
        }

        isRecordVisible = true;
        GameLcdShowResult(InGameStateGetScore(),
                          InGameStateGetMaxCombo());
        return;
    }

    if (WasJoystickMoved()
        && GetJoystickDirection() == JOYSTICK_UP)
    {
        ReadyStateSetStage(GAME_STAGE_1);
        GameStateChange(GAME_STATE_READY);
    }
    else if (WasJoystickMoved()
        && GetJoystickDirection() == JOYSTICK_DOWN)
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
    SoundPlayerStopBgm();
    SoundPlayerStopEffect();
    G_LOG(INFO, "ResultState exited. \r\n");
}

bool ResultStateIsActive(void)
{
    return isActive;
}
