#include "button.h"

/* 버튼 내부 상태 */
static volatile uint32_t g_servo_last_ms = 0;
static volatile uint32_t g_motor_last_ms = 0;

static volatile ButtonEvent_t g_btn_event = BTN_NONE;

void Button_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint32_t now = HAL_GetTick();

    /* ===== 서보 버튼: PC13 ===== */
    if (GPIO_Pin == GPIO_PIN_13)
    {
        // FALLING이면 눌렸을 때 LOW가 맞아야 함
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) != GPIO_PIN_RESET) return;
        if (now - g_servo_last_ms < 200) return;

        g_servo_last_ms = now;
        g_btn_event = BTN_SERVO;
        return;
    }

    /* ===== 모터 버튼: PC2 ===== */
    if (GPIO_Pin == GPIO_PIN_2)
    {
        // FALLING이면 눌렸을 때 LOW가 맞아야 함
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2) != GPIO_PIN_RESET) return;
        if (now - g_motor_last_ms < 200) return;

        g_motor_last_ms = now;
        g_btn_event = BTN_MOTOR;
        return;
    }
}

ButtonEvent_t Button_GetEvent(void)
{
    ButtonEvent_t ev = g_btn_event;
    if (ev != BTN_NONE)
        g_btn_event = BTN_NONE;
    return ev;
}
