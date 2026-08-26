#include "InGameState.h"
#include "GButton.h"
#include "GLed.h"
#include "GCheat.h"
#include <stdlib.h>

#define MOLE_VISIBLE_TIME_MS 800U

static bool isActive;
static uint32_t score;
static uint32_t moleStartTick;
static LedId activeMole;

static void SpawnMole(uint32_t now)
{
    activeMole = (LedId)(rand() % LED_MAX);
    moleStartTick = now;
    SetLedState(activeMole, LED_ON);
}

static void HideMole(void)
{
    if (activeMole < LED_MAX)
    {
        SetLedState(activeMole, LED_OFF);
        activeMole = LED_MAX;
    }
}

void InGameStateEnter(void)
{
    isActive = true;
    score = 0U;
    moleStartTick = HAL_GetTick();
    srand((unsigned int)moleStartTick);
    activeMole = LED_MAX;
    SpawnMole(moleStartTick);
    G_LOG(INFO, "InGameState entered. \r\n");
}

void InGameStateUpdate(void)
{
    if (!isActive)
    {
        return;
    }

    const uint32_t now = HAL_GetTick();

    if ((activeMole < LED_MAX) &&
        WasButtonPressed((ButtonId)activeMole))
    {
        ++score;
        HideMole();
    }
    else if ((activeMole < LED_MAX) &&
             ((now - moleStartTick) >= MOLE_VISIBLE_TIME_MS))
    {
        HideMole();
    }

    if (activeMole == LED_MAX)
    {
        SpawnMole(now);
    }
}

void InGameStateExit(void)
{
    if (!isActive)
    {
        return;
    }

    HideMole();
    isActive = false;
    G_LOG(INFO, "InGameState exited. \r\n");
}

bool InGameStateIsActive(void)
{
    return isActive;
}
