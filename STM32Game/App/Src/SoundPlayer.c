#include "SoundPlayer.h"

#include "GBuzzer.h"
#include "SoundResource.h"

static const SoundSequence *const soundSequences[] =
{
    &scaleSound,
    &startSound,
    &successSound,
    &failSound,
    &buttonSound,
    &titleBgm,
    &canonSound,
};

static const SoundSequence *currentSequence;
static uint32_t currentStepIndex;
static uint32_t nextStepTick;
static bool soundPlaying;

static void StartCurrentStep(void)
{
    const SoundStep *step = &currentSequence->steps[currentStepIndex];

    if (step->note == SOUND_NOTE_REST)
    {
        StopBuzzer();
    }
    else
    {
        PlayFrequency((uint32_t)step->note);
    }

    nextStepTick = HAL_GetTick() + (uint32_t)step->delay;
}

void SoundPlayerInit(void)
{
    GBuzzerInit();
    currentSequence = NULL;
    currentStepIndex = 0U;
    nextStepTick = 0U;
    soundPlaying = false;
}

void SoundPlayerPlay(SoundId soundId)
{
    const uint32_t index = (uint32_t)soundId;

    if (index >= SOUND_ID_MAX_COUNT)
    {
        return;
    }

    SoundPlayerStop();
    currentSequence = soundSequences[index];
    currentStepIndex = 0U;
    soundPlaying = true;
    StartCurrentStep();
}

void SoundPlayerStop(void)
{
    StopBuzzer();
    currentSequence = NULL;
    currentStepIndex = 0U;
    nextStepTick = 0U;
    soundPlaying = false;
}

void SoundPlayerUpdate(void)
{
    if (!soundPlaying || ((int32_t)(HAL_GetTick() - nextStepTick) < 0))
    {
        return;
    }

    ++currentStepIndex;

    if (currentStepIndex >= currentSequence->stepCount)
    {
        if (!currentSequence->loop)
        {
            SoundPlayerStop();
            return;
        }

        currentStepIndex = 0U;
    }

    StartCurrentStep();
}

bool SoundPlayerIsPlaying(void)
{
    return soundPlaying;
}
