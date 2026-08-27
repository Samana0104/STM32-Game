#pragma once

#include "GameStage.h"
#include "main.h"

/* 다음 카운트다운에서 시작할 스테이지를 지정한다. */
void ReadyStateSetStage(GameStage stage);

/* 현재 준비 중이거나 플레이 중인 스테이지를 반환한다. */
GameStage ReadyStateGetStage(void);

void ReadyStateEnter(void);
void ReadyStateUpdate(void);
void ReadyStateExit(void);
bool ReadyStateIsActive(void);
