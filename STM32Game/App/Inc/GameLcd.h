#pragma once

#include "main.h"

/* LCD1602에 게임 시작 메뉴를 표시한다. */
void GameLcdShowTitleMenu(void);

/* LCD1602에 현재 연속 성공 횟수를 표시한다. */
void GameLcdShowCombo(uint32_t combo);

/* 두더지를 놓쳤다는 메시지를 표시한다. */
void GameLcdShowMiss(void);

/* 게임 종료 직후 GameOver 메시지를 표시한다. */
void GameLcdShowGameOver(void);

/* 최고 점수와 최근 게임의 미스 횟수를 표시한다. */
void GameLcdShowRecord(uint32_t bestRecord, uint32_t missCount);
