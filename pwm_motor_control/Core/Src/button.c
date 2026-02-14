#include "button.h"

/* 버튼 내부 상태 (button.c에서만 관리) */
static volatile uint32_t g_btn_last_ms = 0; // 마지막 버튼 입력 시간
static volatile uint8_t  g_btn_event   = 0; // 버튼 이벤트 발생 여부 플래그

void Button_EXTI_Callback(uint16_t GPIO_Pin)
{
#ifdef B1_Pin
    if (GPIO_Pin == B1_Pin)
#else
    if (GPIO_Pin == GPIO_PIN_13)
#endif
    {
        uint32_t now = HAL_GetTick(); // 현재 시간 측정

        /* 디바운스(채터링 방지)
         * - 버튼은 한번 눌러도 접점이 튀면서 짧은 시간에 여러 번 인터럽트가 발생할 수 있음
         * - 마지막 유효 입력 이후 200ms 이내 재발생하면 바운스로 보고 무시
         */
        if (now - g_btn_last_ms < 200) return; // 디바운스

        // 마지막 입력시간 초기화
        g_btn_last_ms = now;

        g_btn_event = 1; // 메인 루프에서 처리
    }
}

uint8_t Button_GetEvent(void)
{
    if (g_btn_event) // 이벤트 플래그가 있다면
    {
        g_btn_event = 0;  // 이벤트 소비
        return 1;
    }
    return 0;
}
