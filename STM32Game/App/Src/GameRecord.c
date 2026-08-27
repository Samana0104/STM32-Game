#include "GameRecord.h"

static uint32_t bestScore;
static uint32_t lastMissCount;

void GameRecordInit(void)
{
    bestScore = 0U;
    lastMissCount = 0U;
}

void GameRecordSave(uint32_t score, uint32_t missCount)
{
    if (score > bestScore)
    {
        bestScore = score;
    }

    lastMissCount = missCount;
}

uint32_t GameRecordGetBestScore(void)
{
    return bestScore;
}

uint32_t GameRecordGetLastMissCount(void)
{
    return lastMissCount;
}
