#ifndef G_LED_H
#define G_LED_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    LED_ID_1 = 0,
    LED_ID_2,
    LED_ID_3,
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