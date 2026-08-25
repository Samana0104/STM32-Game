#include "GameState.h"
#include "InGameState.h"

typedef void (*GameStateUpdateFunction)(void);

static GameState currentState = GAME_STATE_TITLE;
static uint32_t stateStartTick;
static GameStateUpdateFunction currentUpdateFunction;

static bool GameStateIsValid(GameState state)
{
    return (state >= GAME_STATE_TITLE) && (state < GAME_STATE_COUNT);
}

void GameStateInit(GameState initialState)
{
    currentState = GameStateIsValid(initialState)
        ? initialState
        : GAME_STATE_TITLE;
    stateStartTick = HAL_GetTick();

    if (currentState == GAME_STATE_PLAYING)
    {
        currentUpdateFunction = InGameStateUpdate;
        InGameStateEnter();
    }
    else
    {
        currentUpdateFunction = NULL;
    }
}

void GameStateUpdate(void)
{
    if (currentUpdateFunction != NULL)
    {
        currentUpdateFunction();
    }
}

GameState GameStateGet(void)
{
    return currentState;
}

bool GameStateChange(GameState nextState)
{
    if (!GameStateIsValid(nextState))
    {
        return false;
    }

    if (nextState == currentState)
    {
        return true;
    }

    if (currentState == GAME_STATE_PLAYING)
    {
        InGameStateExit();
    }

    currentState = nextState;
    stateStartTick = HAL_GetTick();

    if (currentState == GAME_STATE_PLAYING)
    {
        currentUpdateFunction = InGameStateUpdate;
        InGameStateEnter();
    }
    else
    {
        currentUpdateFunction = NULL;
    }

    return true;
}

uint32_t GameStateGetElapsedMs(void)
{
    return HAL_GetTick() - stateStartTick;
}
