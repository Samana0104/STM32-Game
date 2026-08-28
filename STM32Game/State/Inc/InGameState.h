#pragma once

#include "main.h"

/* 인게임 상태에 진입할 때 한 번 호출한다. */
void InGameStateEnter(void);

/* 인게임 상태가 활성화된 동안 반복 호출한다. */
void InGameStateUpdate(void);

/* 인게임 상태에서 빠져나갈 때 한 번 호출한다. */
void InGameStateExit(void);

/* 현재 인게임 상태가 활성화되어 있는지 반환한다. */
bool InGameStateIsActive(void);

/* 제한 시간이 끝났는지 반환한다. */
bool InGameStateIsFinished(void);

/* 현재 점수를 반환한다. */
uint32_t InGameStateGetScore(void);

/* 현재 연속 성공 횟수를 반환한다. */
uint32_t InGameStateGetCombo(void);

/* 현재 게임에서 두더지를 놓친 횟수를 반환한다. */
uint32_t InGameStateGetMissCount(void);

/* 현재 스테이지의 남은 Life를 반환한다. */
uint8_t InGameStateGetLife(void);
