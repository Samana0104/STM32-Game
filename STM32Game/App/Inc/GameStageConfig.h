#pragma once

#include "GameStage.h"
#include "main.h"

typedef struct
{
    uint32_t durationMs;
    uint32_t moleVisibleMs;
    uint8_t activeMoleCount;
} GameStageConfig;

/* 유효하지 않은 스테이지가 들어오면 Stage 1 설정을 반환한다. */
const GameStageConfig *GameStageGetConfig(GameStage stage);
