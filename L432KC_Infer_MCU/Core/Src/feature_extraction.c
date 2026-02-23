/*
 * feature_extraction.c
 *
 *  Created on: Feb 18, 2026
 *      Author: MSP
 */

#include "feature_extraction.h"
#include <math.h>

// ── 전역 변수 정의 ──
SensorSample window_buf[WINDOW_SIZE];
uint32_t window_idx = 0;
float g_features[FEATURE_NUM];  // 피처 8개

// ── Scaler 값 (Python에서 추출) ──
static const float SCALER_MEAN[FEATURE_NUM] = {
    /* current_mean */ 2.76654572e-01f,
    /* current_std  */ 1.20861058e-01f,
    /* current_p2p  */ 3.35900001e-01f,
    /* voltage_mean */ 1.23016058e+01f,
    /* voltage_std  */ 5.33317645e-02f,
    /* power_mean   */ 3.40752857e+00f,
    /* power_std    */ 1.49845377e+00f,
    /* az_p2p       */ 9.51862857e+03f,
};

static const float SCALER_STD[FEATURE_NUM] = {
    /* current_mean */ 6.49993489e-03f,
    /* current_std  */ 7.51517185e-03f,
    /* current_p2p  */ 1.86564044e-02f,
    /* voltage_mean */ 3.18633904e-03f,
    /* voltage_std  */ 5.50561673e-03f,
    /* power_mean   */ 8.25589299e-02f,
    /* power_std    */ 9.10539362e-02f,
    /* az_p2p       */ 2.27943410e+03f,
};

// ── Feature 추출 ──
void extract_features(SensorSample *buf, float *features) {
    float az_min=1e9f,  az_max=-1e9f;
    float cur_min=1e9f, cur_max=-1e9f;

    float cur_sum=0.0f, vol_sum=0.0f, pwr_sum=0.0f;
    float cur_sq_sum=0.0f, vol_sq_sum=0.0f, pwr_sq_sum=0.0f;

    for(int i = 0; i < WINDOW_SIZE; i++) {
        float az  = buf[i].az;
        float cur = buf[i].current;
        float vol = buf[i].voltage;
        float pwr = buf[i].power;

        // 최솟값/최댓값 갱신
        if(az  < az_min)  az_min  = az;
        if(az  > az_max)  az_max  = az;
        if(cur < cur_min) cur_min = cur;
        if(cur > cur_max) cur_max = cur;

        // 합계 누적
        cur_sum    += cur;
        vol_sum    += vol;
        pwr_sum    += pwr;
        cur_sq_sum += cur * cur;
        vol_sq_sum += vol * vol;
        pwr_sq_sum += pwr * pwr;
    }

    float n = (float)WINDOW_SIZE;

    // 평균 계산
    float current_mean = cur_sum / n;
    float voltage_mean = vol_sum / n;
    float power_mean   = pwr_sum / n;

    // 표준편차 계산 (numpy std ddof=0 동일 방식)
    float cur_var = (cur_sq_sum / n) - (current_mean * current_mean);
    float vol_var = (vol_sq_sum / n) - (voltage_mean * voltage_mean);
    float pwr_var = (pwr_sq_sum / n) - (power_mean   * power_mean);
    if(cur_var < 0.0f) cur_var = 0.0f;
    if(vol_var < 0.0f) vol_var = 0.0f;
    if(pwr_var < 0.0f) pwr_var = 0.0f;
    float current_std = sqrtf(cur_var);
    float voltage_std = sqrtf(vol_var);
    float power_std   = sqrtf(pwr_var);

    // p2p 계산
    float current_p2p = cur_max - cur_min;
    float az_p2p      = az_max  - az_min;

    // 피처 배열 저장 (Python utils.py 순서와 동일)
    features[0] = current_mean;
    features[1] = current_std;
    features[2] = current_p2p;
    features[3] = voltage_mean;
    features[4] = voltage_std;
    features[5] = power_mean;
    features[6] = power_std;
    features[7] = az_p2p;
}

// ── 정규화 (StandardScaler) ──
void normalize(float *features) {
    for(int i = 0; i < FEATURE_NUM; i++) {
        features[i] = (features[i] - SCALER_MEAN[i]) / SCALER_STD[i];
    }
}
