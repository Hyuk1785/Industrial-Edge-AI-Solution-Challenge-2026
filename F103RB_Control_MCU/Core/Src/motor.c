/*
 * motor.c
 *
 * 역할:
 *  - L298N Motor Driver 제어
 *  - GPIO 방향 제어 + PWM 속도 제어
 *
 * 구조:
 *  - Motor_Init()      : 초기화
 *  - Motor_SetSpeed() : 속도/방향 설정
 *  - Motor_Stop()     : 정지
 *
 * 동작 원리:
 *  - IN1 / IN2 = 회전 방향
 *  - ENA(PWM) = 속도
 */

#include "motor.h"

// 모터 타이머 변수
static TIM_HandleTypeDef *motor_tim = NULL;

/* ===== L298N 핀 매핑 ===== */
#define IN1_PORT GPIOB
#define IN1_PIN  GPIO_PIN_0
#define IN2_PORT GPIOB
#define IN2_PIN  GPIO_PIN_1
#define ENA_PWM_CH TIM_CHANNEL_1

// 속도 Duty(0 ~ 100%)를 PWM으로 변경
static void PWM_SetDuty(uint8_t duty)
{
	// 입력값 오류 보호
    if (duty > 100)
    	duty = 100;

    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(motor_tim); // 타이머의 끝 값(주기)
    uint32_t ccr = ((arr + 1U) * duty) / 100U; // PWM High가 끝나는 지점

    // 계산결과 오류 보호
    if (ccr > arr)
    	ccr = arr;

    __HAL_TIM_SET_COMPARE(motor_tim, ENA_PWM_CH, ccr); // 타이머의 CCR값을 설정
}

// 모터 초기화
void Motor_Init(TIM_HandleTypeDef *htim)
{
    motor_tim = htim; // 모터가 사용하는 타이머 설정

    HAL_TIM_PWM_Start(motor_tim, ENA_PWM_CH); // 모터 타이머 PWM 출력을 ON시킴

    Motor_Stop();
}

// 모터 정지
void Motor_Stop(void)
{
	// IN1, IN2를 LOW로 설정해, 정지
    HAL_GPIO_WritePin(IN1_PORT, IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN2_PORT, IN2_PIN, GPIO_PIN_RESET);

    // PWM값 0으로 설정
    PWM_SetDuty(0);
}

// 모터 속도 설정
void Motor_SetSpeed(int16_t speed)
{
	// 입력갑 오류 방지
    if (speed > 100) speed = 100;
    if (speed < -100) speed = -100;

    // 모터 정지
    if (speed == 0)
    {
        Motor_Stop();
        return;
    }

    // 모터가 양의 방향
    if (speed > 0)
    {
        HAL_GPIO_WritePin(IN1_PORT, IN1_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(IN2_PORT, IN2_PIN, GPIO_PIN_RESET);
    }
    else // 음의 방향
    {
        HAL_GPIO_WritePin(IN1_PORT, IN1_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(IN2_PORT, IN2_PIN, GPIO_PIN_SET);
        speed = -speed;
    }

    PWM_SetDuty((uint8_t)speed);
}

