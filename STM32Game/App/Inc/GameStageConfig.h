#pragma once

#include "GameStage.h"
#include "main.h"

typedef struct
{
    uint32_t durationMs;
    uint32_t moleVisibleMinMs;
    uint32_t moleVisibleMaxMs;
    uint8_t maxActiveMoleCount;
    uint8_t initialLife;
} GameStageConfig;

/* 유효하지 않은 스테이지가 들어오면 Stage 1 설정을 반환한다. */
const GameStageConfig *GameStageGetConfig(GameStage stage);
