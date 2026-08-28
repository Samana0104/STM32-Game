#include "GameStageConfig.h"

/* 스테이지별 난이도는 이 배열의 값만 변경해서 조정한다. */
static const GameStageConfig stageConfigs[GAME_STAGE_COUNT] =
{
    [GAME_STAGE_1] = {  20000U, 1500U, 1500U, 1U, 10U },
    [GAME_STAGE_2] = {  25000U, 1000U, 1200U, 2U,  8U },
    [GAME_STAGE_3] = {  30000U,  800U, 1100U, 3U,  7U },
    [GAME_STAGE_4] = {  60000U,  700U, 1100U, 5U,  6U },
    [GAME_STAGE_5] = { 180000U,  600U, 1000U, 7U,  5U }
};

const GameStageConfig *GameStageGetConfig(GameStage stage)
{
    if (stage < GAME_STAGE_1 || stage >= GAME_STAGE_COUNT)
    {
        stage = GAME_STAGE_1;
    }

    return &stageConfigs[stage];
}
