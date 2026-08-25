#pragma once

#include <stdbool.h>
#include <stdint.h>

/* Live Expressions: [0]=button 1, [1]=button 2, [2]=button 3, [3]=start */
extern volatile uint8_t mole_button_raw[4];
extern volatile uint8_t mole_button_stable[4];

void moleInit(void);
void moleRun(void);

bool moleIsRunning(void);
uint32_t moleGetScore(void);
uint8_t moleGetActiveLed(void);
