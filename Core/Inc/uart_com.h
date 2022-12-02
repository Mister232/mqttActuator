/**
  ************************************************************************************************
  * @file           : uart_com.h
  * @brief          : Header for uart_com.c file.
  *                   This file contains the common defines and the function and variable exports
  ************************************************************************************************
*/


#ifndef __UART_COM_H
#define __UART_COM_H


// Defines
#define pc_uart         huart2
#define esp8266_uart     huart1

#define PC_MAX_SENDLEN  1024
#define PC_MAX_RECVLEN  1024
#define ESP_MAX_SENDLEN  1024
#define ESP_MAX_RECVLEN  1024


// Functions
extern void pc_printf(char *fmt, ...);
extern void esp_transmit(char *fmt, ...);


// Variables
extern uint8_t PC_TxBUF[PC_MAX_SENDLEN];
extern uint8_t PC_RxBUF[PC_MAX_RECVLEN];

extern uint8_t ESP_TxBUF[ESP_MAX_SENDLEN];
extern uint8_t ESP_RxBUF[ESP_MAX_RECVLEN];
extern volatile uint8_t ESP_RxLen;
extern volatile uint8_t ESP_RecvEndFlag;


#endif /* __UART_COM_H */
