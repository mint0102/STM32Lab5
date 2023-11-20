/*
 * uart.h
 *
 *  Created on: Nov 20, 2023
 *      Author: HaHuyen
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "main.h"
#include "stdio.h"
#include "string.h"
#include "software_timer.h"

extern uint8_t buffer_flag;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void uart_communication_fsm(void);
void command_parser_fsm(void);


#endif /* INC_UART_H_ */
