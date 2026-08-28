#pragma once
#include "main.h"

typedef enum
{
    GAME_STATE_TITLE = 0,
    GAME_STATE_RECORD,
    GAME_STATE_READY,
    GAME_STATE_PLAYING,
    GAME_STATE_RESULT,
    GAME_STATE_COUNT
} GameState;

/* 게임 상태 모듈을 지정한 초기 상태로 초기화한다. */
void GameStateInit(GameState initialState);

/* 현재 상태에 연결된 업데이트 함수를 호출한다. */
void GameStateUpdate(void);

/* 현재 게임 상태를 반환한다. */
GameState GameStateGet(void);

/* 상태를 변경하고 상태 시작 시간을 갱신한다. */
bool GameStateChange(GameState nextState);
