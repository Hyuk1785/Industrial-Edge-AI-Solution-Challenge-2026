/*
 * ina219.h
 *
 *  Created on: Feb 17, 2026
 *      Author: MSP
 */

#ifndef INC_INA219_H_
#define INC_INA219_H_

#include "main.h"
#include <stdbool.h>

/* I2C 주소 */
#define INA219_ADDR         (0x40 << 1)

/* 레지스터 */
#define INA219_REG_CONFIG   0x00
#define INA219_REG_SHUNT_V  0x01
#define INA219_REG_BUS_V    0x02
#define INA219_REG_POWER    0x03
#define INA219_REG_CURRENT  0x04
#define INA219_REG_CALIB    0x05

/* 데이터 구조체 */
typedef struct {
    float current;   // A (전류)
    float voltage;   // V (전압)
    float power;     // W (전력)
} INA219_t;

/* 함수 선언 */
bool INA219_Init(I2C_HandleTypeDef *hi2c);
void INA219_Read_All(I2C_HandleTypeDef *hi2c, INA219_t *data);

#endif /* INC_INA219_H_ */
