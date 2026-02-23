/*
 * anomaly_detection.c
 *
 *  Created on: Feb 18, 2026
 *      Author: MSP
 */
#include "anomaly_detection.h"
#include <stdio.h>
#include "main.h"  // ← 추가!

extern UART_HandleTypeDef huart1;  // ← F103RB 통신용
extern UART_HandleTypeDef huart2;  // ← PC 디버깅용 (기존)

// ── 전역 변수 ──
float g_output = 0.0f;  // AI 모델 출력값

// ── 초기화 (필요없지만 남겨둠) ──
void anomaly_detection_init(void) {
    g_output = 0.0f;
}

// ── 0 or 1 판단 후 F103RB로 전송 ──
void anomaly_detection_process(float ai_output) {
    // ── 1. 0.5 기준 판단 ──
    uint8_t result = (ai_output > 0.6f) ? 1 : 0;

    // ── 2. PC 디버깅 출력 (UART2) ──
    printf("AI: %.4f -> %d\r\n", ai_output, result);

    // ── 3. F103RB로 전송 (UART1) ──
    uint8_t tx_char = result + '0';  // 0→'0', 1→'1'
    HAL_UART_Transmit(&huart1, &tx_char, 1, 100);
}

