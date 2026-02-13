/*
 * servo.h
 *
 *  Created on: Feb 13, 2026
 *      Author: admin
 */

#ifndef INC_SERVO_H_
#define INC_SERVO_H_

#include "stm32f1xx_hal.h"

/* 초기화 */
void Servo_Init(TIM_HandleTypeDef *htim);

/* 각도 설정 (0~180) */
void Servo_SetAngle(uint8_t deg);



#endif /* INC_SERVO_H_ */
