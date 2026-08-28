#pragma once

#include "main.h"

#define GAME_RECORD_MAX_COUNT 100U

/* Flash에서 내림차순 점수 기록을 불러온다. */
void GameRecordInit(void);

/* 점수가 100위 안에 들면 삽입하고 Flash에 저장한다. */
bool GameRecordSave(uint32_t score);

uint32_t GameRecordGetBestScore(void);
uint32_t GameRecordGetScore(uint32_t rank);
