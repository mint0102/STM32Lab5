/*
 * uart.c
 *
 *  Created on: Nov 20, 2023
 *      Author: HaHuyen
 */

#include "uart.h"

#define MAX_BUFFER_SIZE 30
unsigned char temp = 0;
uint8_t buffer[MAX_BUFFER_SIZE] = "!ADC=";
uint8_t index_buffer = 0;
uint8_t msg[10] = "!ADC=";
uint8_t flagSendCommand = 0;
uint8_t buffer_flag = 0;

#define WAIT_HEADER			0
#define RECEIVE				1
#define RECEIVE_RST_2  		2
#define RECEIVE_RST_3  		3
#define END_RST 			4
#define RECEIVE_OK_2 		5
#define END_OK 				6
uint8_t statusReceive = WAIT_HEADER;

#define WAIT_COMMAND		0
#define SEND_COMMAND		1
#define RESEND_COMMAND		2
uint8_t statusCommand = WAIT_COMMAND;
uint16_t ADC_value = 0;
char str[4] = "0000";

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2){
		HAL_UART_Transmit(&huart2, &temp, 1, 200);
		buffer_flag = 1;
		HAL_UART_Receive_IT(&huart2, &temp, 1);
	}
}

//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
//	if(huart->Instance == USART2){
//
//		//HAL_UART_Transmit(&huart2, &temp, 1, 50);
//		buffer[index_buffer++] = temp;
//		if(index_buffer == 30)
//			index_buffer = 0;
//		buffer_flag = 1;
//		HAL_UART_Receive_IT(&huart2, &temp, 1);
//	}
//}

void uart_communiation_fsm(void) {
	switch(statusCommand) {
	case WAIT_COMMAND:
		if(flagSendCommand){
			statusCommand = SEND_COMMAND;
			//Read ADC
			HAL_ADC_Start(&hadc1);
			ADC_value = HAL_ADC_GetValue(&hadc1);
			// Convert to string and print
			sprintf(str, "%u", ADC_value);
		}
		break;
	case SEND_COMMAND:
		HAL_UART_Transmit(&huart2, (uint8_t*) "\r\n", 1, 10);
		HAL_UART_Transmit(&huart2, msg, 5, 100);
		HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str), 100);
		HAL_UART_Transmit(&huart2, (uint8_t*) "#\r\n", 3, 100);
		if(flagSendCommand){
			statusCommand = RESEND_COMMAND;
			setTimer1(300);
		}
		break;
	case RESEND_COMMAND:
		if (timer1_flag) {
			HAL_UART_Transmit(&huart2, msg, 5, 100);
			HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str), 100);
			HAL_UART_Transmit(&huart2, (uint8_t*) "#\r\n", 3, 100);
			setTimer1(300);
		}
		if(!flagSendCommand){
			statusCommand = WAIT_COMMAND;
		}
		break;
	default:
		statusCommand = WAIT_COMMAND;
	}
}

void command_parser_fsm(void) {
	switch (statusReceive) {
	case WAIT_HEADER:
		if (temp == '!') {
			statusReceive = RECEIVE;
		}
		break;
	case RECEIVE:
		if (temp == 'R')
			statusReceive = RECEIVE_RST_2;
		else if (temp == 'O')
			statusReceive = RECEIVE_OK_2;
		else
			statusReceive = WAIT_HEADER;
		break;
	case RECEIVE_RST_2:
		if (temp == 'S')
			statusReceive = RECEIVE_RST_3;
		else
			statusReceive = WAIT_HEADER;
		break;
	case RECEIVE_RST_3:
		if (temp == 'T')
			statusReceive = END_RST;
		else
			statusReceive = WAIT_HEADER;
		break;
	case RECEIVE_OK_2:
		if (temp == 'K')
			statusReceive = END_OK;
		else
			statusReceive = WAIT_HEADER;
		break;
	case END_RST:
		if (temp == '#') {
			statusReceive = WAIT_HEADER;
			flagSendCommand = 1;
		} else
			flagSendCommand = WAIT_HEADER;
		break;
	case END_OK:
		if (temp == '#') {
			statusReceive = WAIT_HEADER;
			flagSendCommand = 0;
		} else
			statusReceive = WAIT_HEADER;
		break;
	default:
		statusReceive = WAIT_HEADER;
		break;
	}
}
