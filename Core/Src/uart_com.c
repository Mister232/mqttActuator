/**
  *******************************************************************************
  * @file           : uart_com.c
  * @brief          : This file contains functions for communication with pc and
  * 				  ESP8266 module. It also contains some variables which are
  * 				  important for the communication handling in the other files
  ********************************************************************************
*/


// Includes
#include <stdio.h>
#include <stdarg.h>
#include "main.h"
#include "uart_com.h"


// Global variables
uint8_t PC_TxBUF[PC_MAX_SENDLEN];
uint8_t PC_RxBUF[PC_MAX_RECVLEN];

uint8_t ESP_TxBUF[ESP_MAX_SENDLEN];
uint8_t ESP_RxBUF[ESP_MAX_RECVLEN];
volatile uint8_t ESP_RxLen = 0;
volatile uint8_t ESP_RecvEndFlag = 0;



/**
  * @brief  Function to transmit to the pc for debug purpose.
  * @retval None
  */
void pc_printf(char *fmt, ...)
{
	if(DEBUG_MODE == 1)
	{
		uint16_t i;
		va_list ap;

		va_start(ap, fmt);
		vsprintf((char*) PC_TxBUF, fmt, ap);
		va_end(ap);
		i = strlen((const char*) PC_TxBUF);
		HAL_UART_Transmit(&huart2, PC_TxBUF, i, 100);

		memset(PC_TxBUF, 0, PC_MAX_SENDLEN);
	}
}


/**
  * @brief  Function to transmit to the ESP8266 module.
  * @retval None
  */
void esp_transmit(char *fmt, ...)
{
	uint16_t i, j;
	va_list ap;
	va_start(ap, fmt);
	vsprintf((char*) ESP_TxBUF, fmt, ap);
	va_end(ap);

	for (i = 0; i < ESP_MAX_SENDLEN; i++) {
		j = i + 1;
		if (ESP_TxBUF[i] == '\00') {
			for (; j < ESP_MAX_SENDLEN; j++) {
				ESP_TxBUF[j - 1] = ESP_TxBUF[j];
			}
		}
	}

	i = strlen((const char*) ESP_TxBUF);

	HAL_UART_Transmit(&huart1, ESP_TxBUF, i, 100);

	memset(ESP_TxBUF, 0, ESP_MAX_SENDLEN);
	memset(ESP_RxBUF, 0, ESP_MAX_RECVLEN);
	ESP_RecvEndFlag = 0;
}
