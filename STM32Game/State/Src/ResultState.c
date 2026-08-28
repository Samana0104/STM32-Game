#include "ResultState.h"

#include "GameRecord.h"
#include "GameState.h"
#include "GCheat.h"
#include "GJoystick.h"
#include "GLcd1602.h"
#include "InGameState.h"
#include "ReadyState.h"
#include "SoundPlayer.h"

#define GAME_OVER_DISPLAY_MS 2000U
#define LCD_RESULT_MAX       9999U

static bool isActive;
static bool isRecordVisible;
static uint32_t gameOverStartTick;

static uint32_t ClampResultValue(uint32_t value)
{
    return value > LCD_RESULT_MAX ? LCD_RESULT_MAX : value;
}

static void PrintResult(void)
{
    Lcd1602Printf("Your Score:%lu\nMax Combo:%lu",
                  (unsigned long)ClampResultValue(InGameStateGetScore()),
                  (unsigned long)ClampResultValue(InGameStateGetMaxCombo()));
}

void ResultStateEnter(void)
{
    isActive = true;
    isRecordVisible = false;
    gameOverStartTick = HAL_GetTick();

    GameRecordSave(InGameStateGetScore());
    Lcd1602Printf("GameOver");
    SoundPlayerPlayBgm(SOUND_ID_RESULT_BGM);

    if (InGameStateGetLife() == 0U)
    {
        SoundPlayerPlayEffect(SOUND_ID_FAIL);
    }
    else
    {
        SoundPlayerPlayEffect(SOUND_ID_SUCCESS);
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
        PrintResult();
        return;
    }

    if (WasJoystickMoved())
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
