#pragma once

#include "main.h"

/* 전원이 유지되는 동안 사용할 게임 기록을 초기화한다. */
void GameRecordInit(void);

/* 한 게임의 결과를 최고 기록과 최근 미스 횟수에 반영한다. */
void GameRecordSave(uint32_t score, uint32_t missCount);

uint32_t GameRecordGetBestScore(void);
uint32_t GameRecordGetLastMissCount(void);
