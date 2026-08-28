#include "GameScore.h"

GameComboRank GameScoreGetComboRank(uint32_t combo)
{
    if (combo >= 100U)
    {
        return GAME_COMBO_RANK_PERFECT;
    }
    if (combo >= 60U)
    {
        return GAME_COMBO_RANK_GREAT;
    }
    if (combo >= 30U)
    {
        return GAME_COMBO_RANK_NICE;
    }
    if (combo >= 10U)
    {
        return GAME_COMBO_RANK_GOOD;
    }
    return GAME_COMBO_RANK_NONE;
}

uint32_t GameScoreGetComboBonus(GameComboRank rank)
{
    switch (rank)
    {
        case GAME_COMBO_RANK_GOOD:
            return 10U;

        case GAME_COMBO_RANK_NICE:
            return 13U;

        case GAME_COMBO_RANK_GREAT:
            return 16U;

        case GAME_COMBO_RANK_PERFECT:
            return 20U;

        case GAME_COMBO_RANK_NONE:
        default:
            return 0U;
    }
}

uint32_t GameScoreGetHitPoints(uint32_t combo)
{
    return GAME_HIT_BASE_SCORE +
           GameScoreGetComboBonus(GameScoreGetComboRank(combo));
}
