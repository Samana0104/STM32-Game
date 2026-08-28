#pragma once

#include "main.h"

#define GAME_HIT_BASE_SCORE  10U
#define GAME_MISS_PENALTY    20U
#define GAME_SCORE_MAX     9999U

typedef enum
{
    GAME_COMBO_RANK_NONE = 0,
    GAME_COMBO_RANK_GOOD,
    GAME_COMBO_RANK_NICE,
    GAME_COMBO_RANK_GREAT,
    GAME_COMBO_RANK_PERFECT
} GameComboRank;

GameComboRank GameScoreGetComboRank(uint32_t combo);
uint32_t GameScoreGetComboBonus(GameComboRank rank);
uint32_t GameScoreGetHitPoints(uint32_t combo);
