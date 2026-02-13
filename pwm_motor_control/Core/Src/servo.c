#include "servo.h"

/* ===== 내부 변수 ===== */
static TIM_HandleTypeDef *servo_tim = NULL;

/* ===== 내부 함수 ===== */
static void Servo_SetPulseUs(uint16_t us)
{
    if (us < 500)  us = 500;
    if (us > 2500) us = 2500;

    __HAL_TIM_SET_COMPARE(servo_tim, TIM_CHANNEL_1, us);
}

/* ===== API ===== */

void Servo_Init(TIM_HandleTypeDef *htim)
{
    servo_tim = htim;

    HAL_TIM_PWM_Start(servo_tim, TIM_CHANNEL_1);
}

void Servo_SetAngle(uint8_t deg)
{
    if (deg > 180) deg = 180;

    uint16_t us = 500 + (uint16_t)((2000U * deg) / 180U);

    Servo_SetPulseUs(us);
}
