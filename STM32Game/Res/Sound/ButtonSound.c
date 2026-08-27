#include "SoundResource.h"

static const SoundStep steps[] =
{
    {SOUND_NOTE_HIGH_DO, SOUND_DELAY_SHORT},
};

const SoundSequence buttonSound =
{
    steps,
    SOUND_STEP_COUNT(steps),
    false,
};
