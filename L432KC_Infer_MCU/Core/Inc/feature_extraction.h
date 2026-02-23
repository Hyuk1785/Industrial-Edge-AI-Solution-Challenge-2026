/*
 * feature_extraction.h
 *
 *  Created on: Feb 18, 2026
 *      Author: MSP
 */

#ifndef INC_FEATURE_EXTRACTION_H_
#define INC_FEATURE_EXTRACTION_H_

#include <stdint.h>

// ── 설정 ──
#define WINDOW_SIZE  50
#define FEATURE_NUM  8  // 5 → 8

// ── 센서 샘플 구조체 ──
typedef struct {
    float ax, ay, az;
    float gx, gy, gz;
    float current, voltage, power;
} SensorSample;

// ── 전역 변수 (extern) ──
extern SensorSample window_buf[WINDOW_SIZE];
extern uint32_t window_idx;
extern float g_features[FEATURE_NUM];  // 피처 8개

// ── 함수 선언 ──

/**
 * @brief 50개 샘플에서 8개 피처 추출
 * @param buf 윈도우 버퍼 (50개 샘플)
 * @param features 출력 배열 (8개 피처)
 *
 * 추출 피처:
 *   1. current_mean  : 평균 전류
 *   2. current_std   : 전류 표준편차
 *   3. current_p2p   : 전류 peak-to-peak
 *   4. voltage_mean  : 평균 전압
 *   5. voltage_std   : 전압 표준편차
 *   6. power_mean    : 평균 전력
 *   7. power_std     : 전력 표준편차
 *   8. az_p2p        : Z축 가속도 peak-to-peak
 */
void extract_features(SensorSample *buf, float *features);

/**
 * @brief 피처 정규화 (StandardScaler 적용)
 * @param features 정규화할 피처 배열 (8개)
 */
void normalize(float *features);

#endif /* INC_FEATURE_EXTRACTION_H_ */
