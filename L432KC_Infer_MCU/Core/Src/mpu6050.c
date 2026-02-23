/*
 * mpu6050.c
 *
 *  Created on: Feb 14, 2026
 *      Author: MSP
 */


#include "mpu6050.h"
#include <stdio.h>

/* Private variables */
static const uint16_t i2c_timeout = 100;
static I2C_HandleTypeDef *mpu_i2c;

/* Private function prototypes */
static HAL_StatusTypeDef MPU6050_ReadRegister(uint8_t reg, uint8_t *data, uint8_t len);
static HAL_StatusTypeDef MPU6050_WriteRegister(uint8_t reg, uint8_t data);

/**
 * @brief  MPU6050 초기화
 * @param  hi2c: I2C 핸들 포인터
 * @retval true: 성공, false: 실패
 */
bool MPU6050_Init(I2C_HandleTypeDef *hi2c) {
    uint8_t check;
    uint8_t data;

    mpu_i2c = hi2c;

    // 1. WHO_AM_I 체크 (0x68이어야 함)
    if(MPU6050_ReadRegister(WHO_AM_I_REG, &check, 1) != HAL_OK) {
        printf("MPU6050: I2C Read Failed\r\n");
        return false;
    }

    if(check == 0x68) {
        printf("MPU6050: Device Found (0x%02X)\r\n", check);

        // 2. Power Management - Wake up (reset = 0)
        data = 0x00;
        MPU6050_WriteRegister(PWR_MGMT_1_REG, data);

        // 3. Sample Rate Divider (1kHz / (1 + 7) = 125Hz)
        data = 0x07;
        MPU6050_WriteRegister(SMPLRT_DIV_REG, data);

        // 4. Accelerometer Configuration (±2g)
        data = 0x00;
        MPU6050_WriteRegister(ACCEL_CONFIG_REG, data);

        // 5. Gyroscope Configuration (±250°/s)
        data = 0x00;
        MPU6050_WriteRegister(GYRO_CONFIG_REG, data);

        printf("MPU6050: Initialized Successfully\r\n");
        return true;
    }

    printf("MPU6050: Device Not Found (Got 0x%02X)\r\n", check);
    return false;
}

/**
 * @brief  가속도계 데이터 읽기
 * @param  hi2c: I2C 핸들 포인터
 * @param  DataStruct: 데이터 저장 구조체
 */
void MPU6050_Read_Accel(I2C_HandleTypeDef *hi2c, MPU6050_t *DataStruct) {
    uint8_t data[6];

    // 가속도 데이터 읽기 (6 bytes: X_H, X_L, Y_H, Y_L, Z_H, Z_L)
    MPU6050_ReadRegister(ACCEL_XOUT_H_REG, data, 6);

    // RAW 데이터 조합 (Big Endian)
    DataStruct->Accel_X_RAW = (int16_t)(data[0] << 8 | data[1]);
    DataStruct->Accel_Y_RAW = (int16_t)(data[2] << 8 | data[3]);
    DataStruct->Accel_Z_RAW = (int16_t)(data[4] << 8 | data[5]);

    // ±2g 설정: LSB Sensitivity = 16384 LSB/g
    DataStruct->Ax = DataStruct->Accel_X_RAW / 16384.0f;
    DataStruct->Ay = DataStruct->Accel_Y_RAW / 16384.0f;
    DataStruct->Az = DataStruct->Accel_Z_RAW / 16384.0f;
}

/**
 * @brief  자이로스코프 데이터 읽기
 * @param  hi2c: I2C 핸들 포인터
 * @param  DataStruct: 데이터 저장 구조체
 */
void MPU6050_Read_Gyro(I2C_HandleTypeDef *hi2c, MPU6050_t *DataStruct) {
    uint8_t data[6];

    // 자이로 데이터 읽기
    MPU6050_ReadRegister(GYRO_XOUT_H_REG, data, 6);

    // RAW 데이터 조합
    DataStruct->Gyro_X_RAW = (int16_t)(data[0] << 8 | data[1]);
    DataStruct->Gyro_Y_RAW = (int16_t)(data[2] << 8 | data[3]);
    DataStruct->Gyro_Z_RAW = (int16_t)(data[4] << 8 | data[5]);

    // ±250°/s 설정: LSB Sensitivity = 131 LSB/(°/s)
    DataStruct->Gx = DataStruct->Gyro_X_RAW / 131.0f;
    DataStruct->Gy = DataStruct->Gyro_Y_RAW / 131.0f;
    DataStruct->Gz = DataStruct->Gyro_Z_RAW / 131.0f;
}

/**
 * @brief  모든 센서 데이터 읽기 (가속도 + 자이로)
 * @param  hi2c: I2C 핸들 포인터
 * @param  DataStruct: 데이터 저장 구조체
 */
void MPU6050_Read_All(I2C_HandleTypeDef *hi2c, MPU6050_t *DataStruct) {
    MPU6050_Read_Accel(hi2c, DataStruct);
    MPU6050_Read_Gyro(hi2c, DataStruct);
}

/* ========== Private Functions ========== */

/**
 * @brief  레지스터 읽기
 */
static HAL_StatusTypeDef MPU6050_ReadRegister(uint8_t reg, uint8_t *data, uint8_t len) {
    return HAL_I2C_Mem_Read(mpu_i2c, MPU6050_ADDR, reg, 1, data, len, i2c_timeout);
}

/**
 * @brief  레지스터 쓰기
 */
static HAL_StatusTypeDef MPU6050_WriteRegister(uint8_t reg, uint8_t data) {
    return HAL_I2C_Mem_Write(mpu_i2c, MPU6050_ADDR, reg, 1, &data, 1, i2c_timeout);
}
