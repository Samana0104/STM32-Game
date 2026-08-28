#pragma once

#include "GameScore.h"
#include "main.h"

/* 스테이지 번호와 3, 2, 1, START 카운트다운을 표시한다. */
void GameLcdShowCountdown(uint8_t stage, const char *text);

/* LCD1602에 현재 콤보와 Life를 표시한다. */
void GameLcdShowCombo(uint32_t combo, uint8_t life,
                      GameComboRank announcedRank, uint32_t awardedScore);

/* 두더지를 놓쳤을 때 감점과 남은 Life를 표시한다. */
void GameLcdShowMiss(uint8_t life, uint32_t scorePenalty);

/* 게임 종료 직후 GameOver 메시지를 표시한다. */
void GameLcdShowGameOver(void);

/* 방금 끝난 게임의 점수와 최고 연속 콤보를 표시한다. */
void GameLcdShowResult(uint32_t score, uint32_t maxCombo);

/* 한 페이지에 연속된 두 순위와 점수를 표시한다. */
void GameLcdShowRecordPage(uint32_t firstRank, uint32_t firstScore,
                           uint32_t secondRank, uint32_t secondScore);
