#pragma once

#include "SoundPlayer.h"

typedef struct
{
    SoundNote note;
    SoundDelay delay;
} SoundStep;

typedef struct
{
    const SoundStep *steps;
    uint32_t stepCount;
    bool loop;
} SoundSequence;

#define SOUND_STEP_COUNT(steps) (sizeof(steps) / sizeof((steps)[0]))

extern const SoundSequence scaleSound;
extern const SoundSequence startSound;
extern const SoundSequence successSound;
extern const SoundSequence failSound;
extern const SoundSequence buttonSound;
extern const SoundSequence titleBgm;
extern const SoundSequence canonSound;
extern const SoundSequence inGameBgm;
