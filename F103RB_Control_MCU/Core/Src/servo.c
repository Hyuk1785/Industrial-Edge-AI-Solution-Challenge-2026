#include "servo.h"

// 서보모터 타이머 변수
static TIM_HandleTypeDef *servo_tim = NULL;

// 펄스 길이(us)로 각도 설정
static void PWM_SetUs(uint8_t deg)
{
	// 각도 변환
	uint16_t us = 500 + (uint16_t)((2000U * deg) / 180U);

	// 계산결과 오류 보호
    if (us < 500)  us = 500;
    if (us > 2500) us = 2500;

    // 타이머의 us값을 설정
    __HAL_TIM_SET_COMPARE(servo_tim, TIM_CHANNEL_1, us);
}

// 서보 모터 초기화
void Servo_Init(TIM_HandleTypeDef *htim)
{
    servo_tim = htim; // 서보모터가 사용하는 타이머 설정

    HAL_TIM_PWM_Start(servo_tim, TIM_CHANNEL_1); // 서보모터 타이머 PWM 출력을 ON시킴
}

// 사용자가 쓰기 쉬운 함수
void Servo_SetAngle(uint8_t deg)
{
	// 입력 각도 제한
    if (deg > 180)
    	deg = 180;

    PWM_SetUs(deg);
}
