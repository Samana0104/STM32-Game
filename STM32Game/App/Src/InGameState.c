#include "InGameState.h"
#include "GameLcd.h"
#include "GameStageConfig.h"
#include "GButton.h"
#include "GCheat.h"
#include "GFnd.h"
#include "GLed.h"
#include "ReadyState.h"

#define MOLE_COUNT             ((uint8_t)LED_MAX)

static bool isActive;
static bool isFinished;
static uint8_t activeMoles;
static uint32_t startTick;
static uint32_t moleStartTick;
static uint32_t score;
static uint32_t combo;
static uint32_t missCount;
static uint32_t randomState;
static const GameStageConfig *stageConfig;

static uint8_t GetMoleMask(uint8_t moleId)
{
    return (uint8_t)(1U << moleId);
}

static uint8_t CountActiveMoles(void)
{
    uint8_t moleId;
    uint8_t count = 0U;

    for (moleId = 0U; moleId < MOLE_COUNT; moleId++)
    {
        if ((activeMoles & GetMoleMask(moleId)) != 0U)
        {
            count++;
        }
    }

    return count;
}

static void TurnOffAllMoles(void)
{
    uint8_t moleId;

    for (moleId = 0U; moleId < MOLE_COUNT; moleId++)
    {
        if ((activeMoles & GetMoleMask(moleId)) != 0U)
        {
            SetLedState((LedId)moleId, LED_OFF);
        }
    }

    activeMoles = 0U;
}

static uint32_t NextRandom(void)
{
    randomState = (randomState * 1664525U) + 1013904223U;
    return randomState;
}

static void SpawnMoles(uint32_t currentTick)
{
    uint8_t moleIds[MOLE_COUNT];
    uint8_t moleCount = stageConfig->activeMoleCount;
    uint8_t index;

    TurnOffAllMoles();

    if (moleCount == 0U)
    {
        moleCount = 1U;
    }
    else if (moleCount > MOLE_COUNT)
    {
        moleCount = MOLE_COUNT;
    }

    for (index = 0U; index < MOLE_COUNT; index++)
    {
        moleIds[index] = index;
    }

    for (index = 0U; index < moleCount; index++)
    {
        uint8_t selectedIndex;
        uint8_t selectedMole;

        selectedIndex = (uint8_t)(index
            + (NextRandom() % (uint32_t)(MOLE_COUNT - index)));
        selectedMole = moleIds[selectedIndex];
        moleIds[selectedIndex] = moleIds[index];
        moleIds[index] = selectedMole;

        activeMoles |= GetMoleMask(selectedMole);
        SetLedState((LedId)selectedMole, LED_ON);
    }

    moleStartTick = currentTick;
}

static bool IsGameTimeOver(uint32_t currentTick)
{
    return (currentTick - startTick) >= stageConfig->durationMs;
}

void InGameStateEnter(void)
{
    stageConfig = GameStageGetConfig(ReadyStateGetStage());
    startTick = HAL_GetTick();
    moleStartTick = startTick;
    score = 0U;
    combo = 0U;
    missCount = 0U;
    randomState = startTick ^ 0xA5A5A5A5U;
    activeMoles = 0U;
    isFinished = false;
    isActive = true;

    SetFndSingleDigit((uint8_t)ReadyStateGetStage());
    SetFnd4DigitNumber(0U);

    GameLcdShowCombo(combo);

    SpawnMoles(startTick);
    G_LOG(INFO, "InGameState entered.\r\n");
}

void InGameStateUpdate(void)
{
    uint32_t currentTick;
    uint8_t moleId;
    uint8_t hitCount = 0U;

    if (!isActive)
    {
        return;
    }

    currentTick = HAL_GetTick();

    if (IsGameTimeOver(currentTick))
    {
        TurnOffAllMoles();
        isFinished = true;
        return;
    }

    for (moleId = 0U; moleId < MOLE_COUNT; moleId++)
    {
        uint8_t moleMask = GetMoleMask(moleId);

        if (((activeMoles & moleMask) != 0U)
            && WasButtonPressed((ButtonId)moleId))
        {
            SetLedState((LedId)moleId, LED_OFF);
            activeMoles &= (uint8_t)(~moleMask);
            hitCount++;
        }
    }

    if (hitCount > 0U)
    {
        score += hitCount;
        combo += hitCount;
        SetFnd4DigitNumber((uint16_t)score);
        GameLcdShowCombo(combo);

        if (activeMoles == 0U)
        {
            SpawnMoles(currentTick);
        }
    }
    else if ((currentTick - moleStartTick) >= stageConfig->moleVisibleMs)
    {
        missCount += CountActiveMoles();
        combo = 0U;
        GameLcdShowMiss();
        SpawnMoles(currentTick);
    }
}

void InGameStateExit(void)
{
    if (!isActive)
    {
        return;
    }

    TurnOffAllMoles();
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
