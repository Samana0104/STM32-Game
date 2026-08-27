#include "InGameState.h"
#include "GameLcd.h"
#include "GButton.h"
#include "GCheat.h"
#include "GFnd.h"
#include "GLed.h"
#include "ReadyState.h"

#define MOLE_COUNT             ((uint8_t)LED_MAX)
#define GAME_DURATION_MS       30000U
#define MOLE_VISIBLE_MS        800U
#define NO_ACTIVE_MOLE         MOLE_COUNT

static bool isActive;
static bool isFinished;
static uint8_t currentMole = NO_ACTIVE_MOLE;
static uint32_t startTick;
static uint32_t moleStartTick;
static uint32_t score;
static uint32_t combo;
static uint32_t missCount;
static uint32_t randomState;

static void TurnOffCurrentMole(void)
{
    if (currentMole < MOLE_COUNT)
    {
        SetLedState((LedId)currentMole, LED_OFF);
        currentMole = NO_ACTIVE_MOLE;
    }
}

static uint32_t NextRandom(void)
{
    randomState = (randomState * 1664525U) + 1013904223U;
    return randomState;
}

static void SpawnMole(uint32_t currentTick)
{
    uint8_t previousMole = currentMole;
    uint8_t nextMole;

    TurnOffCurrentMole();
    nextMole = (uint8_t)(NextRandom() % MOLE_COUNT);

    if (nextMole == previousMole)
    {
        nextMole = (uint8_t)((nextMole + 1U) % MOLE_COUNT);
    }

    currentMole = nextMole;
    moleStartTick = currentTick;
    SetLedState((LedId)currentMole, LED_ON);
}

static bool IsGameTimeOver(uint32_t currentTick)
{
    return (currentTick - startTick) >= GAME_DURATION_MS;
}

void InGameStateEnter(void)
{
    startTick = HAL_GetTick();
    moleStartTick = startTick;
    score = 0U;
    combo = 0U;
    missCount = 0U;
    randomState = startTick ^ 0xA5A5A5A5U;
    currentMole = NO_ACTIVE_MOLE;
    isFinished = false;
    isActive = true;

    SetFndSingleDigit((uint8_t)ReadyStateGetStage());
    SetFnd4DigitNumber(0U);

    GameLcdShowCombo(combo);

    SpawnMole(startTick);
    G_LOG(INFO, "InGameState entered.\r\n");
}

void InGameStateUpdate(void)
{
    uint32_t currentTick;

    if (!isActive)
    {
        return;
    }

    currentTick = HAL_GetTick();

    if (IsGameTimeOver(currentTick))
    {
        TurnOffCurrentMole();
        isFinished = true;
        return;
    }

    if (currentMole < MOLE_COUNT
        && WasButtonPressed((ButtonId)currentMole))
    {
        score++;
        combo++;
        SetFnd4DigitNumber((uint16_t)score);
        GameLcdShowCombo(combo);
        SpawnMole(currentTick);
    }
    else if ((currentTick - moleStartTick) >= MOLE_VISIBLE_MS)
    {
        missCount++;
        combo = 0U;
        GameLcdShowMiss();
        SpawnMole(currentTick);
    }
}

void InGameStateExit(void)
{
    if (!isActive)
    {
        return;
    }

    TurnOffCurrentMole();
    isActive = false;
    G_LOG(INFO, "InGameState exited.\r\n");
}

bool InGameStateIsActive(void)
{
    return isActive;
}

bool InGameStateIsFinished(void)
{
    return isFinished;
}

uint32_t InGameStateGetScore(void)
{
    return score;
}

uint32_t InGameStateGetCombo(void)
{
    return combo;
}

uint32_t InGameStateGetMissCount(void)
{
    return missCount;
}
