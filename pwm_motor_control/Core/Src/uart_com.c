#include "uart_com.h"
#include "led.h"
#include "buzzer.h"

/* ===== 내부 변수 ===== */
static UART_HandleTypeDef *uart_handle = NULL;
static uint8_t rx_data;

static uint8_t one_count = 0;
static uint8_t alarm_triggered = 0;

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

    if (rx_data == '0')
    {
        one_count = 0;
        alarm_triggered = 0;
        LED_SetState(LED_GREEN);
    }
    else if (rx_data == '1')
    {
        one_count++;

        if (one_count >= 5)
        {
            LED_SetState(LED_RED);

            if (!alarm_triggered)
            {
                Buzzer_Beep(500);
                alarm_triggered = 1;
            }
        }
        else
        {
            LED_SetState(LED_YELLOW);
        }
    }

    HAL_UART_Receive_IT(uart_handle, &rx_data, 1);
}
