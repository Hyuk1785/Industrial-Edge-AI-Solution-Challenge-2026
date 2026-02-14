/*
 * mpu6050.h
 *
 *  Created on: Feb 14, 2026
 *      Author: MSP
 */


#ifndef INC_MPU6050_H_
#define INC_MPU6050_H_

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* MPU6050 I2C Address */
#define MPU6050_ADDR        0xD0  // AD0 = GND

/* MPU6050 Registers */
#define WHO_AM_I_REG        0x75
#define PWR_MGMT_1_REG      0x6B
#define SMPLRT_DIV_REG      0x19
#define ACCEL_CONFIG_REG    0x1C
#define ACCEL_XOUT_H_REG    0x3B
#define GYRO_CONFIG_REG     0x1B
#define GYRO_XOUT_H_REG     0x43

/* MPU6050 Data Structure */
typedef struct {
    int16_t Accel_X_RAW;
    int16_t Accel_Y_RAW;
    int16_t Accel_Z_RAW;

    int16_t Gyro_X_RAW;
    int16_t Gyro_Y_RAW;
    int16_t Gyro_Z_RAW;

    float Ax;  // g 단위
    float Ay;
    float Az;

    float Gx;  // °/s 단위
    float Gy;
    float Gz;
} MPU6050_t;

/* Function Prototypes */
bool MPU6050_Init(I2C_HandleTypeDef *hi2c);
void MPU6050_Read_Accel(I2C_HandleTypeDef *hi2c, MPU6050_t *DataStruct);
void MPU6050_Read_Gyro(I2C_HandleTypeDef *hi2c, MPU6050_t *DataStruct);
void MPU6050_Read_All(I2C_HandleTypeDef *hi2c, MPU6050_t *DataStruct);


#endif /* INC_MPU6050_H_ */
