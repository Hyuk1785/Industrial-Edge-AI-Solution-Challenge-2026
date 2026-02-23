#ifndef LED_H
#define LED_H

#include "main.h"

typedef enum
{
    LED_OFF,
    LED_GREEN,
    LED_YELLOW,
    LED_RED
} LedState;

void LED_Init(void);
void LED_SetState(LedState state);

#endif
