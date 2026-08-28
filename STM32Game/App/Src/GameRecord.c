#include "GameRecord.h"
#include "GData.h"

#include <string.h>

static uint32_t bestScores[GAME_RECORD_MAX_COUNT];

static uint32_t FindInsertIndex(uint32_t score)
{
    uint32_t left = 0U;
    uint32_t right = GAME_RECORD_MAX_COUNT;

    /* 같은 점수 뒤에 삽입되는 내림차순 upper bound 탐색이다. */
    while (left < right)
    {
        const uint32_t middle = left + ((right - left) / 2U);
        if (bestScores[middle] >= score)
        {
            left = middle + 1U;
        }
        else
        {
            right = middle;
        }
    }

    return left;
}

void GameRecordInit(void)
{
    LoadFlashData();
    if (!GDataRead(0U, bestScores, sizeof(bestScores)))
    {
        memset(bestScores, 0, sizeof(bestScores));
    }
}

bool GameRecordSave(uint32_t score)
{
    if (score == 0U)
    {
        return false;
    }

    const uint32_t insertIndex = FindInsertIndex(score);
    if (insertIndex >= GAME_RECORD_MAX_COUNT)
    {
        return false;
    }

    const size_t moveCount = GAME_RECORD_MAX_COUNT - insertIndex - 1U;
    if (moveCount > 0U)
    {
        memmove(&bestScores[insertIndex + 1U],
                &bestScores[insertIndex],
                moveCount * sizeof(bestScores[0]));
    }
    bestScores[insertIndex] = score;

    return GDataWrite(0U, bestScores, sizeof(bestScores)) && SaveFlashData();
}

uint32_t GameRecordGetBestScore(void)
{
    return bestScores[0];
}

uint32_t GameRecordGetScore(uint32_t rank)
{
    if (rank == 0U || rank > GAME_RECORD_MAX_COUNT)
    {
        return 0U;
    }

    return bestScores[rank - 1U];
}
