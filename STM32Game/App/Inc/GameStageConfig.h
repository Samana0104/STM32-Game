#pragma once

#include "main.h"

typedef enum
{
    GAME_STAGE_1 = 1,
    GAME_STAGE_2,
    GAME_STAGE_3,
    GAME_STAGE_4,
    GAME_STAGE_5,
    GAME_STAGE_COUNT
} GameStage;

typedef struct
{
    uint32_t durationMs;
    uint32_t moleVisibleMinMs;
    uint32_t moleVisibleMaxMs;
    uint32_t moleSpawnDelayMinMs;
    uint32_t moleSpawnDelayMaxMs;
    uint8_t maxActiveMoleCount;
    uint8_t initialLife;
} GameStageConfig;

/* 유효하지 않은 스테이지가 들어오면 Stage 1 설정을 반환한다. */
const GameStageConfig *GameStageGetConfig(GameStage stage);
