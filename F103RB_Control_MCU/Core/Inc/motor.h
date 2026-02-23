/*
 * motor.h
 *
 *  Created on: Feb 13, 2026
 *      Author: admin
 */

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#include "stm32f1xx_hal.h"

/* 초기화 */
void Motor_Init(TIM_HandleTypeDef *htim);

/* 속도 설정 (-100 ~ 100) */
void Motor_SetSpeed(int16_t speed);

/* 정지 */
void Motor_Stop(void);

#endif /* INC_MOTOR_H_ */
