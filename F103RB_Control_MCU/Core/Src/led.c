#include "led.h"

/* ===== 내부 함수 ===== */
static void LED_Write(uint8_t r, uint8_t g)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0,
                      r ? GPIO_PIN_RESET : GPIO_PIN_SET);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1,
                      g ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

/* ===== 초기화 ===== */
void LED_Init(void)
{
    LED_Write(0,0);
}

/* ===== 상태 설정 ===== */
void LED_SetState(LedState state)
{
    switch(state)
    {
        case LED_OFF:
            LED_Write(0,0);
            break;

        case LED_GREEN:
            LED_Write(0,1);
            break;

        case LED_YELLOW:   // RED + GREEN
            LED_Write(1,1);
            break;

        case LED_RED:
            LED_Write(1,0);
            break;
    }
}
