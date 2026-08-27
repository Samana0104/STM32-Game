#include "ReadyState.h"

#include "GameLcd.h"
#include "GameState.h"
#include "GCheat.h"
#include "GFnd.h"

#define COUNTDOWN_STEP_MS    1000U
#define COUNTDOWN_STEP_COUNT 4U

static const char *const countdownText[COUNTDOWN_STEP_COUNT] =
{
    "3",
    "2",
    "1",
    "START!"
};

static bool isActive;
static GameStage currentStage = GAME_STAGE_1;
static uint8_t currentStep;
static uint32_t countdownStartTick;

static bool IsStageValid(GameStage stage)
{
    return stage >= GAME_STAGE_1 && stage <= GAME_STAGE_5;
}

void ReadyStateSetStage(GameStage stage)
{
    currentStage = IsStageValid(stage) ? stage : GAME_STAGE_1;
}

GameStage ReadyStateGetStage(void)
{
    return currentStage;
}

void ReadyStateEnter(void)
{
    isActive = true;
    currentStep = 0U;
    countdownStartTick = HAL_GetTick();

    SetFndSingleDigit((uint8_t)currentStage);
    GameLcdShowCountdown((uint8_t)currentStage, countdownText[currentStep]);
    G_LOG(INFO, "ReadyState entered.\r\n");
}

void ReadyStateUpdate(void)
{
    uint32_t elapsed;
    uint8_t nextStep;

    if (!isActive)
    {
        return;
    }

    elapsed = HAL_GetTick() - countdownStartTick;
    nextStep = (uint8_t)(elapsed / COUNTDOWN_STEP_MS);

    if (nextStep >= COUNTDOWN_STEP_COUNT)
    {
        GameStateChange(GAME_STATE_PLAYING);
        return;
    }

    if (nextStep != currentStep)
    {
        currentStep = nextStep;
        GameLcdShowCountdown((uint8_t)currentStage,
                             countdownText[currentStep]);
    }
}

void ReadyStateExit(void)
{
    if (!isActive)
    {
        return;
    }

    isActive = false;
    G_LOG(INFO, "ReadyState exited.\r\n");
}

bool ReadyStateIsActive(void)
{
    return isActive;
}
