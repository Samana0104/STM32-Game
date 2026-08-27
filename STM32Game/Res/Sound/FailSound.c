#include "SoundResource.h"

static const SoundStep steps[] =
{
    {SOUND_NOTE_MI, SOUND_DELAY_NORMAL},
    {SOUND_NOTE_RE, SOUND_DELAY_NORMAL},
    {SOUND_NOTE_DO, SOUND_DELAY_VERY_LONG},
};

const SoundSequence failSound =
{
    steps,
    SOUND_STEP_COUNT(steps),
    false,
};
