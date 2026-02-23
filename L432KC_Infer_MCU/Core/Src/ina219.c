/*
 * ina219.c
 *
 *  Created on: Feb 17, 2026
 *      Author: MSP
 */


#include "ina219.h"
#include <stdio.h>

static I2C_HandleTypeDef *ina_i2c;

/* 내부 함수 */
static HAL_StatusTypeDef Write_Reg(uint8_t reg, uint16_t val) {
    uint8_t buf[2] = { val >> 8, val & 0xFF };
    return HAL_I2C_Mem_Write(ina_i2c, INA219_ADDR, reg, 1, buf, 2, 100);
}

static HAL_StatusTypeDef Read_Reg(uint8_t reg, uint16_t *val) {
    uint8_t buf[2];
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(ina_i2c, INA219_ADDR, reg, 1, buf, 2, 100);
    if(ret == HAL_OK) *val = (buf[0] << 8) | buf[1];
    return ret;
}

/* 초기화 */
bool INA219_Init(I2C_HandleTypeDef *hi2c) {
    ina_i2c = hi2c;

    if(Write_Reg(INA219_REG_CONFIG, 0x399F) != HAL_OK) {
        printf("INA219: Init Failed!\r\n");
        return false;
    }

    Write_Reg(INA219_REG_CALIB, 4096);

    printf("INA219: Init OK\r\n");
    return true;
}

/* 전압 읽기 */
static float Read_Voltage(void) {
    uint16_t raw;
    if(Read_Reg(INA219_REG_BUS_V, &raw) != HAL_OK) return 0.0f;
    return ((raw >> 3) * 4) / 1000.0f;
}

/* 전류 읽기 */
static float Read_Current(void) {
    uint16_t raw;
    if(Read_Reg(INA219_REG_CURRENT, &raw) != HAL_OK) return 0.0f;
    return (int16_t)raw * 0.0001f;
}

/* 전력 읽기 */
static float Read_Power(void) {
    uint16_t raw;
    if(Read_Reg(INA219_REG_POWER, &raw) != HAL_OK) return 0.0f;
    return raw * 0.002f;
}

/* 전체 읽기 */
void INA219_Read_All(I2C_HandleTypeDef *hi2c, INA219_t *data) {
    data->voltage = Read_Voltage();
    data->current = Read_Current();
    data->power   = Read_Power();
}
