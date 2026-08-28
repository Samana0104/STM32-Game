#include "SoundPlayer.h"

#include "GBuzzer.h"
#include "SoundResource.h"

typedef struct
{
    const SoundSequence *sequence;
    uint32_t stepIndex;
    uint32_t nextStepTick;
    bool playing;
} SoundPlayerState;

static const SoundSequence *const soundSequences[] =
{
    &scaleSound,
    &startSound,
    &successSound,
    &failSound,
    &buttonSound,
    &titleBgm,
    &canonSound,
    &inGameBgm,
};

static SoundPlayerState bgmState;
static SoundPlayerState effectState;

static void StartCurrentStep(SoundPlayerState *state, BuzzerOutput output)
{
    const SoundStep *step = &state->sequence->steps[state->stepIndex];

    if (step->note == SOUND_NOTE_REST)
    {
        StopBuzzer(output);
    }
    else
    {
        PlayFrequency(output, (uint32_t)step->note);
    }

    state->nextStepTick = HAL_GetTick() + (uint32_t)step->delay;
}

static void StopSound(SoundPlayerState *state, BuzzerOutput output)
{
    StopBuzzer(output);
    state->sequence = NULL;
    state->stepIndex = 0U;
    state->nextStepTick = 0U;
    state->playing = false;
}

static void PlaySound(SoundPlayerState *state, BuzzerOutput output,
                      SoundId soundId)
{
    const uint32_t index = (uint32_t)soundId;

    if (index >= SOUND_ID_MAX_COUNT)
    {
        return;
    }

    StopSound(state, output);
    state->sequence = soundSequences[index];
    state->stepIndex = 0U;
    state->playing = true;
    StartCurrentStep(state, output);
}

static void UpdateSound(SoundPlayerState *state, BuzzerOutput output,
                        uint32_t now)
{
    if (!state->playing || ((int32_t)(now - state->nextStepTick) < 0))
    {
        return;
    }

    ++state->stepIndex;

    if (state->stepIndex >= state->sequence->stepCount)
    {
        if (!state->sequence->loop)
        {
            StopSound(state, output);
            return;
        }

        state->stepIndex = 0U;
    }

    StartCurrentStep(state, output);
}

void SoundPlayerInit(void)
{
    StopSound(&bgmState, BUZZER_OUTPUT_SOUND);
    StopSound(&effectState, BUZZER_OUTPUT_EFFECT_SOUND);
}

void SoundPlayerPlayBgm(SoundId soundId)
{
    PlaySound(&bgmState, BUZZER_OUTPUT_SOUND, soundId);
}

void SoundPlayerPlayEffect(SoundId soundId)
{
    PlaySound(&effectState, BUZZER_OUTPUT_EFFECT_SOUND, soundId);
}

void SoundPlayerStopBgm(void)
{
    StopSound(&bgmState, BUZZER_OUTPUT_SOUND);
}

void SoundPlayerStopEffect(void)
{
    StopSound(&effectState, BUZZER_OUTPUT_EFFECT_SOUND);
}

void SoundPlayerUpdate(void)
{
    const uint32_t now = HAL_GetTick();
    UpdateSound(&bgmState, BUZZER_OUTPUT_SOUND, now);
    UpdateSound(&effectState, BUZZER_OUTPUT_EFFECT_SOUND, now);
}

bool SoundPlayerIsBgmPlaying(void)
{
    return bgmState.playing;
}

bool SoundPlayerIsEffectPlaying(void)
{
    return effectState.playing;
}
