#ifndef __BUTTON_H
#define __BUTTON_H

#include "stm32f1xx_hal.h"

void Button_EXTI_Callback(uint16_t GPIO_Pin);
uint8_t Button_GetEvent(void);

#endif
