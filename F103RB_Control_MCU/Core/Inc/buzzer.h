#ifndef BUZZER_H
#define BUZZER_H

#include "main.h"

void Buzzer_Init(TIM_HandleTypeDef *htim);

void Buzzer_On(void);
void Buzzer_Off(void);

void Buzzer_Beep(uint16_t ms);
void Buzzer_Task(void);

#endif
