/*
 * motor.c
 *
 *  Created on: Feb 13, 2026
 *      Author: admin
 */

#include "motor.h"

/* ===== 내부 static 변수 ===== */
static TIM_HandleTypeDef *motor_tim = NULL;

/* ===== L298N 핀 매핑 ===== */
//#define IN3_PORT GPIOB
//#define IN3_PIN  GPIO_PIN_0
//#define IN4_PORT GPIOB
//#define IN4_PIN  GPIO_PIN_1
//
//#define IN1_PORT GPIOB
//#define IN1_PIN  GPIO_PIN_10
//#define IN2_PORT GPIOB
//#define IN2_PIN  GPIO_PIN_11
//
//#define ENB_PWM_CH TIM_CHANNEL_1
//#define ENA_PWM_CH TIM_CHANNEL_2


#define IN1_PORT GPIOB
#define IN1_PIN  GPIO_PIN_0   // IN3

#define IN2_PORT GPIOB
#define IN2_PIN  GPIO_PIN_1   // IN4

#define ENA_PWM_CH TIM_CHANNEL_1

/* ===== 내부 PWM 함수 ===== */
static void PWM_SetDuty(uint8_t duty)
{
    if (duty > 100) duty = 100;

    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(motor_tim);
    uint32_t ccr = ((arr + 1U) * duty) / 100U;

    if (ccr > arr) ccr = arr;

    __HAL_TIM_SET_COMPARE(motor_tim, ENA_PWM_CH, ccr);
}

/* ===== API ===== */

void Motor_Init(TIM_HandleTypeDef *htim)
{
    motor_tim = htim;

    HAL_TIM_PWM_Start(motor_tim, ENA_PWM_CH);

    Motor_Stop();
}

void Motor_Stop(void)
{
    HAL_GPIO_WritePin(IN1_PORT, IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN2_PORT, IN2_PIN, GPIO_PIN_RESET);
    PWM_SetDuty(0);
}

void Motor_SetSpeed(int16_t speed)
{
    if (speed > 100) speed = 100;
    if (speed < -100) speed = -100;

    if (speed == 0)
    {
        Motor_Stop();
        return;
    }

    if (speed > 0)
    {
        HAL_GPIO_WritePin(IN1_PORT, IN1_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(IN2_PORT, IN2_PIN, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(IN1_PORT, IN1_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(IN2_PORT, IN2_PIN, GPIO_PIN_SET);
        speed = -speed;
    }

    PWM_SetDuty((uint8_t)speed);
}

