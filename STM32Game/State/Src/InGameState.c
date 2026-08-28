#include "InGameState.h"
#include "GameLcd.h"
#include "GameScore.h"
#include "GameStageConfig.h"
#include "GButton.h"
#include "GCheat.h"
#include "GFnd.h"
#include "GLed.h"
#include "ReadyState.h"
#include "SoundPlayer.h"

#define MOLE_COUNT ((uint8_t)LED_MAX)

static bool isActive;
static bool isFinished;
static uint8_t activeMoles;
static uint8_t life;
static uint32_t startTick;
static uint32_t moleStartTick;
static uint32_t moleVisibleDurationMs;
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

static uint32_t GetNextMoleVisibleDuration(void)
{
    uint32_t minimum = stageConfig->moleVisibleMinMs;
    uint32_t maximum = stageConfig->moleVisibleMaxMs;

    if (maximum <= minimum)
    {
        return minimum;
    }

    return minimum + (NextRandom() % (maximum - minimum + 1U));
}

static void AddScore(uint32_t points)
{
    if (points >= (GAME_SCORE_MAX - score))
    {
        score = GAME_SCORE_MAX;
        return;
    }
    score += points;
}

static void SubtractScore(uint32_t penalty)
{
    score = penalty >= score ? 0U : score - penalty;
}

static void SpawnMoles(uint32_t currentTick)
{
    uint8_t moleIds[MOLE_COUNT];
    uint8_t maxMoleCount = stageConfig->maxActiveMoleCount;
    uint8_t moleCount;
    uint8_t index;

    TurnOffAllMoles();

    if (maxMoleCount == 0U)
    {
        maxMoleCount = 1U;
    }
    else if (maxMoleCount > MOLE_COUNT)
    {
        maxMoleCount = MOLE_COUNT;
    }

    moleCount = (uint8_t)(1U + (NextRandom() % maxMoleCount));

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
    moleVisibleDurationMs = GetNextMoleVisibleDuration();
}

static bool IsGameTimeOver(uint32_t currentTick)
{
    return (currentTick - startTick) >= stageConfig->durationMs;
}

void InGameStateEnter(void)
{
    GameStage currentStage = ReadyStateGetStage();

    stageConfig = GameStageGetConfig(currentStage);
    life = stageConfig->initialLife;
    startTick = HAL_GetTick();
    moleStartTick = startTick;

    if (currentStage == GAME_STAGE_1)
    {
        score = 0U;
        combo = 0U;
        missCount = 0U;
    }

    randomState = startTick ^ 0xA5A5A5A5U;
    activeMoles = 0U;
    isFinished = false;
    isActive = true;

    SetFndSingleDigit((uint8_t)currentStage);
    SetFnd4DigitNumber((uint16_t)score);

    GameLcdShowCombo(combo, life, GAME_COMBO_RANK_NONE, 0U);

    SpawnMoles(startTick);
    SoundPlayerPlayBgm(SOUND_ID_IN_GAME_BGM);
    G_LOG(INFO, "InGameState entered.\r\n");
}

void InGameStateUpdate(void)
{
    uint32_t currentTick;
    uint32_t awardedScore = 0U;
    uint32_t announcedScore = 0U;
    uint8_t moleId;
    uint8_t hitCount = 0U;
    GameComboRank announcedRank = GAME_COMBO_RANK_NONE;

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
            combo++;
            awardedScore = GameScoreGetHitPoints(combo);
            AddScore(awardedScore);
            if ((combo % 10U) == 0U)
            {
                announcedRank = GameScoreGetComboRank(combo);
                announcedScore = awardedScore;
            }
            hitCount++;
        }
    }

    if (hitCount > 0U)
    {
        SetFnd4DigitNumber((uint16_t)score);
        GameLcdShowCombo(combo, life, announcedRank, announcedScore);
        SoundPlayerPlayEffect(SOUND_ID_SUCCESS);

        if (activeMoles == 0U)
        {
            SpawnMoles(currentTick);
        }
    }
    else if ((currentTick - moleStartTick) >= moleVisibleDurationMs)
    {
        uint8_t missedMoles = CountActiveMoles();
        uint32_t scorePenalty = (uint32_t)missedMoles * GAME_MISS_PENALTY;

        missCount += missedMoles;
        life = missedMoles >= life ? 0U : (uint8_t)(life - missedMoles);
        SubtractScore(scorePenalty);
        combo = 0U;
        SetFnd4DigitNumber((uint16_t)score);
        GameLcdShowMiss(life, scorePenalty);
        SoundPlayerPlayEffect(SOUND_ID_FAIL);

        if (life == 0U)
        {
            TurnOffAllMoles();
            isFinished = true;
            return;
        }
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
    SoundPlayerStopBgm();
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

uint8_t InGameStateGetLife(void)
{
    return life;
}
