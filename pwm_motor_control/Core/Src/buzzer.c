#include "buzzer.h"

/* ===== 내부 변수 ===== */
static TIM_HandleTypeDef *buzzer_tim = NULL;
static uint32_t buz_arr = 0;

static uint8_t  buz_active = 0;
static uint32_t buz_off_tick = 0;

/* ===== 초기화 ===== */
void Buzzer_Init(TIM_HandleTypeDef *htim)
{
    buzzer_tim = htim;

    // PWM 시작
    HAL_TIM_PWM_Start(buzzer_tim, TIM_CHANNEL_1);

    // ARR 값 저장 (Duty 계산용)
    buz_arr = __HAL_TIM_GET_AUTORELOAD(buzzer_tim);

    // 초기 OFF
    __HAL_TIM_SET_COMPARE(buzzer_tim, TIM_CHANNEL_1, 0);
}

/* ===== ON ===== */
void Buzzer_On(void)
{
    if (!buzzer_tim) return;

    // 50% duty
    __HAL_TIM_SET_COMPARE(buzzer_tim,
                          TIM_CHANNEL_1,
                          (buz_arr + 1) / 2);
}

/* ===== OFF ===== */
void Buzzer_Off(void)
{
    if (!buzzer_tim) return;

    __HAL_TIM_SET_COMPARE(buzzer_tim,
                          TIM_CHANNEL_1,
                          0);
}

/* ===== 일정 시간 삑 ===== */
void Buzzer_Beep(uint16_t ms)
{
    Buzzer_On();

    buz_active   = 1;
    buz_off_tick = HAL_GetTick() + ms;
}

/* ===== main loop에서 호출 ===== */
void Buzzer_Task(void)
{
    if (buz_active)
    {
        if ((int32_t)(HAL_GetTick() - buz_off_tick) >= 0)
        {
            Buzzer_Off();
            buz_active = 0;
        }
    }
}
