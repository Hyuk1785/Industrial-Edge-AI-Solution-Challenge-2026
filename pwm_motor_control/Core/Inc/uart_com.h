#ifndef UART_COM_H
#define UART_COM_H

#include "main.h"

void UART_Com_Init(UART_HandleTypeDef *huart);
void UART_Com_RxCallback(UART_HandleTypeDef *huart);

#endif
