#include "uart_com.h"
#include "led.h"
#include "buzzer.h"
#include <string.h>

/* ===== main.c의 전역 변수 사용 ===== */
extern char current_status[20];
extern uint8_t vote_count;
extern uint8_t defect_count;

/* ===== 내부 변수 ===== */
static UART_HandleTypeDef *uart_handle = NULL;
static uint8_t rx_data;

static uint8_t alarm_triggered = 0;
static uint8_t history[5] = {0};
static uint8_t hist_idx = 0;

/* ===== 초기화 ===== */
void UART_Com_Init(UART_HandleTypeDef *huart)
{
    uart_handle = huart;
    HAL_UART_Receive_IT(uart_handle, &rx_data, 1);
}

/* ===== UART 수신 callback ===== */
void UART_Com_RxCallback(UART_HandleTypeDef *huart)
{
    if (huart != uart_handle) return;

    if (rx_data == '0' || rx_data == '1')
    {
        history[hist_idx % 5] = (rx_data == '1') ? 1 : 0;
        hist_idx++;

        uint8_t cnt = 0;
        for(int i = 0; i < 5; i++)
        	cnt += history[i];

        vote_count = cnt;

        if(cnt >= 3)
        {
            LED_SetState(LED_RED);
            if(!alarm_triggered)
            {
                Buzzer_Beep(500);
                alarm_triggered = 1;
                defect_count++;
                strcpy(current_status, "FAULT");
            }
        }
        else
        {
        	alarm_triggered = 0;

        	if (rx_data == '0') // 정상 상태일 때
        	{
        		LED_SetState(LED_GREEN);
        		strcpy(current_status, "NORMAL");
        	}
        	else // 이상 상태일 때
        	{
        		LED_SetState(LED_YELLOW);
        		strcpy(current_status, "DETECTING");
        	}
        }

    }

    HAL_UART_Receive_IT(uart_handle, &rx_data, 1);
}
