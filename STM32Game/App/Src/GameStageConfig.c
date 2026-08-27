#include "GameStageConfig.h"

#define DEFAULT_STAGE_DURATION_MS    20000U
#define DEFAULT_MOLE_VISIBLE_MS        500U

/* 스테이지별 난이도는 이 배열의 값만 변경해서 조정한다. */
static const GameStageConfig stageConfigs[GAME_STAGE_COUNT] =
{
    [GAME_STAGE_1] = { DEFAULT_STAGE_DURATION_MS, DEFAULT_MOLE_VISIBLE_MS, 1U },
    [GAME_STAGE_2] = { DEFAULT_STAGE_DURATION_MS, DEFAULT_MOLE_VISIBLE_MS, 2U },
    [GAME_STAGE_3] = { DEFAULT_STAGE_DURATION_MS, DEFAULT_MOLE_VISIBLE_MS, 3U },
    [GAME_STAGE_4] = { DEFAULT_STAGE_DURATION_MS, DEFAULT_MOLE_VISIBLE_MS, 4U },
    [GAME_STAGE_5] = { DEFAULT_STAGE_DURATION_MS, DEFAULT_MOLE_VISIBLE_MS, 5U }
};

const GameStageConfig *GameStageGetConfig(GameStage stage)
{
    if (stage < GAME_STAGE_1 || stage >= GAME_STAGE_COUNT)
    {
        stage = GAME_STAGE_1;
    }

    return &stageConfigs[stage];
}
