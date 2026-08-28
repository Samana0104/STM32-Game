#include "GameState.h"
#include "InGameState.h"
#include "ReadyState.h"
#include "ResultState.h"
#include "TitleState.h"

typedef void (*GameStateFunction)(void);

typedef struct
{
    GameStateFunction enter;
    GameStateFunction update;
    GameStateFunction exit;
} GameStateHandler;

static const GameStateHandler stateHandlers[GAME_STATE_COUNT] =
{
    [GAME_STATE_TITLE]   = { TitleStateEnter, TitleStateUpdate, TitleStateExit },
    [GAME_STATE_READY]   = { ReadyStateEnter,
                             ReadyStateUpdate,
                             ReadyStateExit },
    [GAME_STATE_PLAYING] = { InGameStateEnter, InGameStateUpdate, InGameStateExit },
    [GAME_STATE_RESULT]  = { ResultStateEnter,
                             ResultStateUpdate,
                             ResultStateExit }
};

static GameState currentState = GAME_STATE_TITLE;

static bool GameStateIsValid(GameState state)
{
    return (state >= GAME_STATE_TITLE) && (state < GAME_STATE_COUNT);
}

void GameStateInit(GameState initialState)
{
    currentState = GameStateIsValid(initialState) ? initialState : GAME_STATE_TITLE;
    if (stateHandlers[currentState].enter != NULL)
    {
        stateHandlers[currentState].enter();
    }
}

void GameStateUpdate(void)
{
    if (stateHandlers[currentState].update != NULL)
    {
        stateHandlers[currentState].update();
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

    if (stateHandlers[currentState].exit != NULL)
    {
        stateHandlers[currentState].exit();
    }

    currentState = nextState;

    if (stateHandlers[currentState].enter != NULL)
    {
        stateHandlers[currentState].enter();
    }

    return true;
}
