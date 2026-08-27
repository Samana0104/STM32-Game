#include "SoundResource.h"

static const SoundStep steps[] =
{
    {SOUND_NOTE_DO, SOUND_DELAY_SHORT},
    {SOUND_NOTE_MI, SOUND_DELAY_SHORT},
    {SOUND_NOTE_SOL, SOUND_DELAY_NORMAL},
};

const SoundSequence startSound =
{
    steps,
    SOUND_STEP_COUNT(steps),
    false,
};
