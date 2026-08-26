#ifndef G_LED_H
#define G_LED_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    LED_ID_1 = 0, // Q0 (15번 핀)
    LED_ID_2,     // Q1 (1번 핀)
    LED_ID_3,     // Q2 (2번 핀)
    LED_ID_4,     // Q3 (3번 핀)
    LED_ID_5,     // Q4 (4번 핀)
    LED_ID_6,     // Q5 (5번 핀)
    LED_ID_7,     // Q6 (6번 핀)
    LED_MAX
} LedId;

typedef enum
{
    LED_OFF = 0,
    LED_ON = 1
} LedState;

void GledInit(void);
void SetLedState(LedId id, LedState state);

#endif /* G_LED_H */