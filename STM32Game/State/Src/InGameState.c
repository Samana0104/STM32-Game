#include "InGameState.h"
#include "GameScore.h"
#include "GameStageConfig.h"
#include "GameState.h"
#include "GButton.h"
#include "GCheat.h"
#include "GFnd.h"
#include "GLed.h"
#include "GLcd1602.h"
#include "ReadyState.h"
#include "SoundPlayer.h"

#define MOLE_COUNT             ((uint8_t)LED_MAX)
#define STAGE_END_GRACE_MS     2000U
#define LCD_LIFE_MAX           10U

static bool isActive;
static bool isFinished;
static bool isStageEnding;
static bool isWaitingForMole;
static uint8_t activeMoles;
static uint8_t life;
static uint32_t startTick;
static uint32_t moleStartTick;
static uint32_t moleVisibleDurationMs;
static uint32_t nextMoleTick;
static uint32_t score;
static uint32_t combo;
static uint32_t maxCombo;
static uint32_t missCount;
static uint32_t randomState;
static const GameStageConfig *stageConfig;

static void FormatLifeSymbols(uint8_t currentLife,
                              char symbols[LCD_LIFE_MAX + 1U])
{
    const uint8_t symbolCount =
        currentLife > LCD_LIFE_MAX ? LCD_LIFE_MAX : currentLife;

    memset(symbols, '$', symbolCount);
    symbols[symbolCount] = '\0';
}

static const char *GetComboRankText(GameComboRank rank)
{
    switch (rank)
    {
        case GAME_COMBO_RANK_GOOD:    return "Good";
        case GAME_COMBO_RANK_NICE:    return "Nice";
        case GAME_COMBO_RANK_GREAT:   return "Great";
        case GAME_COMBO_RANK_PERFECT: return "Perfect";
        case GAME_COMBO_RANK_NONE:
        default:                      return "";
    }
}

static void PrintCombo(GameComboRank rank, uint32_t awardedScore)
{
    char lifeSymbols[LCD_LIFE_MAX + 1U];

    FormatLifeSymbols(life, lifeSymbols);
    if (rank != GAME_COMBO_RANK_NONE)
    {
        Lcd1602Printf("%s +%lu\nLife: %s", GetComboRankText(rank),
                      (unsigned long)awardedScore, lifeSymbols);
        return;
    }

    Lcd1602Printf("%lu Combo\nLife: %s", (unsigned long)combo,
                  lifeSymbols);
}

static void PrintMiss(uint32_t scorePenalty)
{
    char lifeSymbols[LCD_LIFE_MAX + 1U];

    FormatLifeSymbols(life, lifeSymbols);
    Lcd1602Printf("Miss -%lu\nLife: %s", (unsigned long)scorePenalty,
                  lifeSymbols);
}

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

static uint32_t GetNextMoleSpawnDelay(void)
{
    const uint32_t minimum = stageConfig->moleSpawnDelayMinMs;
    const uint32_t maximum = stageConfig->moleSpawnDelayMaxMs;

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
    isWaitingForMole = false;
}

static void ScheduleNextMoles(uint32_t currentTick)
{
    TurnOffAllMoles();
    isWaitingForMole = true;
    nextMoleTick = currentTick + GetNextMoleSpawnDelay();
}

static bool IsGameTimeOver(uint32_t currentTick)
{
    return (currentTick - startTick) >= stageConfig->durationMs;
}

static void FinishCurrentStage(void)
{
    const GameStage currentStage = ReadyStateGetStage();

    isFinished = true;
    if (life == 0U || currentStage >= GAME_STAGE_5)
    {
        GameStateChange(GAME_STATE_RESULT);
        return;
    }

    ReadyStateSetStage((GameStage)(currentStage + 1));
    GameStateChange(GAME_STATE_READY);
}

static bool IsStageEndingSoon(uint32_t currentTick)
{
    uint32_t elapsed = currentTick - startTick;

    if (elapsed >= stageConfig->durationMs)
    {
        return true;
    }

    return (stageConfig->durationMs - elapsed) <= STAGE_END_GRACE_MS;
}

static bool ApplyMisses(uint8_t missAmount)
{
    uint32_t scorePenalty = (uint32_t)missAmount * GAME_MISS_PENALTY;

    missCount += missAmount;
    life = missAmount >= life ? 0U : (uint8_t)(life - missAmount);
    SubtractScore(scorePenalty);
    combo = 0U;
    SetFnd4DigitNumber((uint16_t)score);
    PrintMiss(scorePenalty);
    SoundPlayerPlayEffect(SOUND_ID_FAIL);

    if (life == 0U)
    {
        TurnOffAllMoles();
        FinishCurrentStage();
        return true;
    }

    return false;
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
        maxCombo = 0U;
        missCount = 0U;
    }

    randomState = startTick ^ 0xA5A5A5A5U;
    activeMoles = 0U;
    isFinished = false;
    isStageEnding = false;
    isWaitingForMole = false;
    isActive = true;

    SetFndSingleDigit((uint8_t)currentStage);
    SetFnd4DigitNumber((uint16_t)score);

    PrintCombo(GAME_COMBO_RANK_NONE, 0U);

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
    uint8_t wrongPressCount = 0U;
    GameComboRank announcedRank = GAME_COMBO_RANK_NONE;

    if (!isActive)
    {
        return;
    }

    currentTick = HAL_GetTick();

    if (IsGameTimeOver(currentTick))
    {
        TurnOffAllMoles();
        FinishCurrentStage();
        return;
    }

    if (IsStageEndingSoon(currentTick))
    {
        if (!isStageEnding)
        {
            TurnOffAllMoles();
            isStageEnding = true;
        }
        return;
    }

    if (isWaitingForMole)
    {
        for (moleId = 0U; moleId < MOLE_COUNT; ++moleId)
        {
            if (WasButtonPressed((ButtonId)moleId))
            {
                ++wrongPressCount;
            }
        }

        if (wrongPressCount > 0U)
        {
            if (!ApplyMisses(wrongPressCount))
            {
                ScheduleNextMoles(currentTick);
            }
            return;
        }

        if ((int32_t)(currentTick - nextMoleTick) >= 0)
        {
            SpawnMoles(currentTick);
        }
        return;
    }

    for (moleId = 0U; moleId < MOLE_COUNT; moleId++)
    {
        uint8_t moleMask = GetMoleMask(moleId);

        if (!WasButtonPressed((ButtonId)moleId))
        {
            continue;
        }

        if ((activeMoles & moleMask) != 0U)
        {
            SetLedState((LedId)moleId, LED_OFF);
            activeMoles &= (uint8_t)(~moleMask);
            combo++;
            if (combo > maxCombo)
            {
                maxCombo = combo;
            }
            awardedScore = GameScoreGetHitPoints(combo);
            AddScore(awardedScore);
            if ((combo % 10U) == 0U)
            {
                announcedRank = GameScoreGetComboRank(combo);
                announcedScore = awardedScore;
            }
            hitCount++;
        }
        else
        {
            wrongPressCount++;
        }
    }

    if (wrongPressCount > 0U)
    {
        if (ApplyMisses(wrongPressCount))
        {
            return;
        }

        ScheduleNextMoles(currentTick);
    }
    else if (hitCount > 0U)
    {
        SetFnd4DigitNumber((uint16_t)score);
        PrintCombo(announcedRank, announcedScore);
        SoundPlayerPlayEffect(SOUND_ID_SUCCESS);

        if (activeMoles == 0U)
        {
            ScheduleNextMoles(currentTick);
        }
    }
    else if ((currentTick - moleStartTick) >= moleVisibleDurationMs)
    {
        uint8_t missedMoles = CountActiveMoles();

        if (ApplyMisses(missedMoles))
        {
            return;
        }

        ScheduleNextMoles(currentTick);
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

uint32_t InGameStateGetMaxCombo(void)
{
    return maxCombo;
}

uint32_t InGameStateGetMissCount(void)
{
    return missCount;
}

uint8_t InGameStateGetLife(void)
{
    return life;
}
