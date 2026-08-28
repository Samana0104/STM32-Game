#pragma once

#include "main.h"

typedef enum
{
    SOUND_NOTE_REST = 0,
    SOUND_NOTE_LOW_RE = 147,
    SOUND_NOTE_LOW_FA = 175,
    SOUND_NOTE_LOW_SOL = 196,
    SOUND_NOTE_LOW_LA = 220,
    SOUND_NOTE_LOW_SI = 247,
    SOUND_NOTE_DO = 262,
    SOUND_NOTE_RE = 294,
    SOUND_NOTE_MI = 330,
    SOUND_NOTE_FA = 349,
    SOUND_NOTE_SOL = 392,
    SOUND_NOTE_LA = 440,
    SOUND_NOTE_SI = 494,
    SOUND_NOTE_HIGH_DO = 523,
    SOUND_NOTE_HIGH_RE = 587,
    SOUND_NOTE_HIGH_MI = 659,
    SOUND_NOTE_HIGH_FA = 698,
    SOUND_NOTE_HIGH_SOL = 784,
    SOUND_NOTE_HIGH_LA = 880,
    SOUND_NOTE_HIGH_SI = 988
} SoundNote;

typedef enum
{
    SOUND_DELAY_SHORT = 100,
    SOUND_DELAY_NORMAL = 200,
    SOUND_DELAY_LONG = 400,
    SOUND_DELAY_HALF_SECOND = 500,
    SOUND_DELAY_VERY_LONG = 800,
    SOUND_DELAY_ONE_SECOND = 1000
} SoundDelay;

typedef enum
{
    SOUND_ID_SCALE = 0,
    SOUND_ID_START,
    SOUND_ID_SUCCESS,
    SOUND_ID_FAIL,
    SOUND_ID_BUTTON,
    SOUND_ID_TITLE_BGM,
    SOUND_ID_CANON,
    SOUND_ID_IN_GAME_BGM,
    SOUND_ID_MAX_COUNT
} SoundId;

void SoundPlayerInit(void);
void SoundPlayerPlayBgm(SoundId soundId);
void SoundPlayerPlayEffect(SoundId soundId);
void SoundPlayerStopBgm(void);
void SoundPlayerStopEffect(void);
void SoundPlayerUpdate(void);
bool SoundPlayerIsBgmPlaying(void);
bool SoundPlayerIsEffectPlaying(void);
