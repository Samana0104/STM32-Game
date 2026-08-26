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

