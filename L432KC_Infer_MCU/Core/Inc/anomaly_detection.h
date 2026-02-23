/*
 * anomaly_detection.h
 *
 *  Created on: Feb 18, 2026
 *      Author: MSP
 */

#ifndef INC_ANOMALY_DETECTION_H_
#define INC_ANOMALY_DETECTION_H_

#include <stdint.h>

/* ═══════════════════════════════════════════════
   전역 변수
   ═══════════════════════════════════════════════ */
extern float g_output;  // AI 모델 출력 (0.0~1.0)

/* ═══════════════════════════════════════════════
   함수 선언
   ═══════════════════════════════════════════════ */
/**
 * @brief AI 출력값 판단 후 F103RB로 전송
 * @param ai_output AI 모델 출력값 (0.0~1.0)
 *
 * 동작:
 *   1. ai_output > 0.5 → 1
 *   2. ai_output <= 0.5 → 0
 *   3. UART1로 '0' 또는 '1' 전송
 */
void anomaly_detection_process(float ai_output);

/**
 * @brief 초기화 (선택)
 */
void anomaly_detection_init(void);

#endif
