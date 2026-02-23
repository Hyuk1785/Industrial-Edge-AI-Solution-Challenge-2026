#ifndef __BUTTON_H
#define __BUTTON_H

#include "stm32f1xx_hal.h"

typedef enum {
    BTN_NONE = 0,
    BTN_SERVO,   // PC13
    BTN_MOTOR    // PC2
} ButtonEvent_t;

void Button_EXTI_Callback(uint16_t GPIO_Pin);
ButtonEvent_t Button_GetEvent(void);

#endif
