#include "SoundResource.h"

static const SoundStep steps[] =
{
    {SOUND_NOTE_SOL, SOUND_DELAY_SHORT},
    {SOUND_NOTE_SI, SOUND_DELAY_SHORT},
    {SOUND_NOTE_HIGH_DO, SOUND_DELAY_LONG},
};

const SoundSequence successSound =
{
    steps,
    SOUND_STEP_COUNT(steps),
    false,
};
