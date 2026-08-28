#include "CreditState.h"

#include "GameState.h"
#include "GCheat.h"
#include "GJoystick.h"
#include "SoundPlayer.h"

#define CREDIT_COUNT 4U

static bool isActive;
static uint8_t selectedCredit;

static void PrintCredits(void)
{
    static const char *const labels[CREDIT_COUNT] =
    {
        "1. Hanbit Byeon",
        "2. Sunho Kim",
        "3. Minsik Kim",
        "4. GiBeom Nam",
    };
    const uint8_t firstCredit = (uint8_t)((selectedCredit / 2U) * 2U);

    Lcd1602Printf("%c%s\n%c%s",
                  selectedCredit == firstCredit ? '>' : ' ',
                  labels[firstCredit],
                  selectedCredit == (firstCredit + 1U) ? '>' : ' ',
                  labels[firstCredit + 1U]);
}

void CreditStateEnter(void)
{
    isActive = true;
    selectedCredit = 0U;
    SoundPlayerPlayBgm(SOUND_ID_CREDIT_BGM);
    PrintCredits();
    G_LOG(INFO, "CreditState entered.\r\n");
}

void CreditStateUpdate(void)
{
    if (!isActive || !WasJoystickMoved())
    {
        return;
    }

    const JoystickDirection direction = GetJoystickDirection();

    if (direction == JOYSTICK_RIGHT)
    {
        if (selectedCredit < (CREDIT_COUNT - 1U))
        {
            ++selectedCredit;
            PrintCredits();
            SoundPlayerPlayEffect(SOUND_ID_BUTTON);
        }
    }
    else if (direction == JOYSTICK_LEFT)
    {
        if (selectedCredit > 0U)
        {
            --selectedCredit;
            PrintCredits();
            SoundPlayerPlayEffect(SOUND_ID_BUTTON);
        }
    }
    else if (direction == JOYSTICK_DOWN)
    {
        SoundPlayerPlayEffect(SOUND_ID_BUTTON);
        GameStateChange(GAME_STATE_TITLE);
    }
}

void CreditStateExit(void)
{
    if (!isActive)
    {
        return;
    }

    isActive = false;
    SoundPlayerStopBgm();
    G_LOG(INFO, "CreditState exited.\r\n");
}

bool CreditStateIsActive(void)
{
    return isActive;
}
