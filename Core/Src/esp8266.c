/**
  *********************************************************************************
  * @file           : esp8266.c
  * @brief          : This file contains functions for handling the ESP8266 module
  *********************************************************************************
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "esp8266.h"
#include "uart_com.h"
#include "net_conf.h"


static WIFI_StateTypeDef wifi_state = _OFFLINE;
static WIFI_StateTypeDef trans_state = _UNKNOWN_STATE;
static uint8_t wifi_config_step = 0;


/**
  * @brief  Function to check ACK of ESP8266 module.
  * @param str: Expected ACK of module
  * @retval ACK result
  */
WIFI_StateTypeDef esp8266_CheckRespond(uint8_t *str)
{
	pc_printf("(ESP8266 recv:\r\n%s\r\n", ESP_RxBUF);

	if (strstr((const char*) ESP_RxBUF, (const char*) str) != NULL)
	{
		pc_printf("Match succeed\r\n");
		memset(ESP_RxBUF, 0, ESP_MAX_RECVLEN);
		return _MATCHOK;
	}

	memset(ESP_RxBUF, 0, ESP_MAX_RECVLEN);
	return _MATCHERROR;
}


/**
  * @brief  Function to transmit commands to ESP8266 module.
  * @param cmd: Buffer with command
  * @param ack: Expected ACK of module
  * @param waitms: Time to wait for module response
  * @param newline: Newline expected or not
  * @retval Transmission result
  */
WIFI_StateTypeDef esp8266_TransmitCmd(uint8_t *cmd, uint8_t *ack, uint32_t waitms, uint8_t newline)
{
	int timeout = waitms;
	uint8_t check = 0;

	// Reset receive buffer
	memset(ESP_RxBUF, 0, ESP_MAX_RECVLEN);

	pc_printf("\r\nTry to send cmd: %s\r\n", cmd);

	// Transmit command to module with or without newline
	if (newline == 0)
		esp_transmit("%s", cmd);
	else
		esp_transmit("%s\r\n", cmd);

	pc_printf("Waiting reply\r\n");

	while (timeout--)
	{
		// Check if response was received
		if (ESP_RecvEndFlag == 1)
		{
			check = esp8266_CheckRespond(ack);
			if (check == _MATCHOK)
			{
				pc_printf("Command closed loop completed\r\n");
			}

			// Reset receive variables and interrupt
			ESP_RxLen = 0;
			ESP_RecvEndFlag = 0;
			HAL_UART_Receive_DMA(&huart1, ESP_RxBUF, ESP_MAX_RECVLEN);

			break;
		}

		HAL_Delay(1);
	}

	if (check == _MATCHERROR)
	{
		pc_printf("\r\nCmd match failed\r\n");
		return check;
	}
	if (timeout <= 0)
	{
		pc_printf("Finish waiting\r\n");
		pc_printf("\r\nTimeout\r\n");
		return _TIMEOUT;
	}

	pc_printf("Succeed\r\n");
	return _SUCCEED;
}


/**
  * @brief  Function to reset ESP8266 module.
  * @param waitms: Time to wait for response of module after reset
  * @retval Reset result
  */
WIFI_StateTypeDef esp8266_HardwareReset(uint32_t waitms)
{
	int timeout = waitms;
	WIFI_RST_Enable();
	HAL_Delay(500);
	WIFI_RST_Disable();

	while (timeout--)
	{
		if (ESP_RecvEndFlag == 1)
		{
			pc_printf("Hardware Reset OK!\r\n");
			HAL_Delay(2000);

			ESP_RxLen = 0;
			ESP_RecvEndFlag = 0;
			HAL_UART_Receive_DMA(&huart1, ESP_RxBUF, ESP_MAX_RECVLEN);

			return _SUCCEED;
		}

		HAL_Delay(1);
	}

	if (timeout <= 0)
	{
		pc_printf("Finish waiting\r\n");
		pc_printf("\r\nTimeout\r\n");
		return _TIMEOUT;
	}

	return _UNKNOWN_ERROR;
}


/**
  * @brief  Function to connect to access point with ESP8266 module.
  * @retval Connection state
  */
WIFI_StateTypeDef esp8266_ConnectAP()
{
	uint16_t cmd_len = strlen(AP_SSID) + strlen(AP_PSWD) + 30;
	uint8_t *cmd = (uint8_t*) malloc(cmd_len * sizeof(uint8_t));

	memset(cmd, 0, cmd_len);

	sprintf((char*) cmd, "AT+CWJAP_CUR=\"%s\",\"%s\"", AP_SSID, AP_PSWD);

	if (esp8266_TransmitCmd(cmd, (uint8_t*) "WIFI CONNECTED", 3 * ESP8266_MAX_TIMEOUT, WITH_NEWLINE) == _SUCCEED)
		wifi_state = _ONLINE;
	else
		wifi_state = _OFFLINE;

	return wifi_state;
}


/**
  * @brief  Function to connect to server ESP8266 module.
  * @retval Connection state
  */
WIFI_StateTypeDef esp8266_ConnectServer()
{
	uint16_t cmd_len = strlen(IpServer) + strlen(ServerPort) + 30;
	uint8_t *cmd = (uint8_t*) malloc(cmd_len * sizeof(uint8_t));

	memset(cmd, 0, cmd_len);

	sprintf((char*) cmd, "AT+CIPSTART=\"TCP\",\"%s\",%s", IpServer, ServerPort);

	if (esp8266_TransmitCmd(cmd, (uint8_t*) "CONNECT", 3 * ESP8266_MAX_TIMEOUT, WITH_NEWLINE) == _SUCCEED)
		wifi_state = _CONNECTED;
	else
		wifi_state = _DISCONNECTED;

	return wifi_state;
}


/**
  * @brief  Function to set up a TCP connection with ESP8266 module.
  * @retval Connection state
  */
WIFI_StateTypeDef esp8266_SetUpTCPConnection()
{
	uint8_t retry_count = 0;

	// Reset esp8266
	pc_printf("Trying to reset esp8266\r\n");
	wifi_config_step++;

	while (esp8266_HardwareReset(500) != _SUCCEED)
	{
		retry_count++;
		HAL_Delay(100);

		if (retry_count > ESP8266_MAX_RETRY_TIME)
		{
			pc_printf("Reset failed\r\n");
			retry_count = 0;
			trans_state = _UNKNOWN_STATE;
			wifi_config_step--;
			return _FAILED;
		}
	}

	HAL_Delay(100);
	retry_count = 0;		// reset retry count


	// Disable transparent transmission
	pc_printf("Trying to close transparent transmission\r\n");
	wifi_config_step++;

	while (esp8266_TransmitCmd(TRANS_QUIT_CMD, TRANS_QUIT_CMD, ESP8266_MAX_TIMEOUT, WITHOUT_NEWLINE) != _SUCCEED)
	{
		retry_count++;
		HAL_Delay(100);

		if (retry_count > ESP8266_MAX_RETRY_TIME)
		{
			pc_printf("Close transparent transmission failed\r\n");
			retry_count = 0;
			wifi_config_step--;
			return _FAILED;
		}
	}

	trans_state = _TRANS_DISABLE;
	HAL_Delay(100);
	retry_count = 0;		// reset retry count


	// Close echo
	pc_printf("Trying to close echo\r\n");
	wifi_config_step++;

	while (esp8266_TransmitCmd((uint8_t*) "ATE0", OK_ACK, ESP8266_MAX_TIMEOUT, WITH_NEWLINE) != _SUCCEED)
	{
		retry_count++;
		HAL_Delay(100);

		if (retry_count > ESP8266_MAX_RETRY_TIME)
		{
			pc_printf("Close echo failed\r\n");
			retry_count = 0;
			wifi_config_step--;
			return _FAILED;
		}
	}

	HAL_Delay(100);
	retry_count = 0;		// reset retry count


	// Set wifi mode 0:AP 1:STA 2:AP+STA
	pc_printf("Trying to set Wifi mode\r\n");
	wifi_config_step++;

	while (esp8266_TransmitCmd((uint8_t*) "AT+CWMODE_CUR=1", OK_ACK, 1000, WITH_NEWLINE) != _SUCCEED)
	{
		retry_count++;
		HAL_Delay(100);

		if (retry_count > ESP8266_MAX_RETRY_TIME)
		{
			pc_printf("Set Wifi mode failed\r\n");
			retry_count = 0;
			wifi_config_step--;
			return _FAILED;
		}
	}

	HAL_Delay(100);
	retry_count = 0;		// reset retry count


	// Disable auto connect
	pc_printf("Trying to close auto connect\r\n");
	wifi_config_step++;

	while (esp8266_TransmitCmd((uint8_t*) "AT+CWAUTOCONN=0", OK_ACK, 1000, WITH_NEWLINE) != _SUCCEED)
	{
		retry_count++;
		HAL_Delay(100);

		if (retry_count > ESP8266_MAX_RETRY_TIME)
		{
			pc_printf("Close auto connect failed\r\n");
			retry_count = 0;
			wifi_config_step--;
			return _FAILED;
		}
	}

	HAL_Delay(100);
	retry_count = 0;		// reset retry count


	// Connect to AP(Wifi)
	pc_printf("Trying to connect to AP\r\n");
	wifi_config_step++;

	while (esp8266_ConnectAP() != _ONLINE)
	{
		retry_count++;
		HAL_Delay(100);

		if (retry_count > ESP8266_MAX_RETRY_TIME)
		{
			pc_printf("Connect to AP failed\r\n");
			retry_count = 0;
			wifi_config_step--;
			return _FAILED;
		}
	}

	HAL_Delay(100);
	retry_count = 0;		// reset retry count


	// Try to get AP info
	if (wifi_state == _ONLINE)
	{
		while (esp8266_TransmitCmd((uint8_t*) "AT+CWJAP_CUR?", OK_ACK, ESP8266_MAX_TIMEOUT, WITH_NEWLINE) != _SUCCEED)
		{
			retry_count++;
			HAL_Delay(100);

			if (retry_count > ESP8266_MAX_RETRY_TIME / 2)
			{
				pc_printf("Get AP msg failed\r\n");
				pc_printf("Connect server process will not be terminated");
				retry_count = 0;
				wifi_config_step--;
				break;
			}
		}
	}

	HAL_Delay(100);
	retry_count = 0;		// reset retry count


	// Try to get IP info
	if (wifi_state == _ONLINE)
	{
		while (esp8266_TransmitCmd((uint8_t*) "	AT+CIPSTA_CUR?", OK_ACK, ESP8266_MAX_TIMEOUT, WITH_NEWLINE) != _SUCCEED)
		{
			retry_count++;
			HAL_Delay(100);

			if (retry_count > ESP8266_MAX_RETRY_TIME / 2)
			{
				pc_printf("Get IP info failed\r\n");
				pc_printf("(DBG:) Connect server process will not be terminated");
				retry_count = 0;
				wifi_config_step--;
				break;
			}
		}
	}

	HAL_Delay(100);
	retry_count = 0;		// reset retry count


	// Set DHCP
	pc_printf("Trying to set DHCP mode\r\n");
	wifi_config_step++;

	while (esp8266_TransmitCmd((uint8_t*) "AT+CWDHCP_CUR=1,1", OK_ACK, 1000, WITH_NEWLINE) != _SUCCEED)
	{
		retry_count++;
		HAL_Delay(100);

		if (retry_count > ESP8266_MAX_RETRY_TIME)
		{
			pc_printf("Set DHCP model failed\r\n");
			retry_count = 0;
			wifi_config_step--;
			return _FAILED;
		}
	}

	HAL_Delay(100);
	retry_count = 0;


	// Set single connection
	pc_printf("Trying to set single connection\r\n");
	wifi_config_step++;

	while (esp8266_TransmitCmd((uint8_t*) "AT+CIPMUX=0", OK_ACK, 1000, WITH_NEWLINE) != _SUCCEED)
	{
		retry_count++;
		HAL_Delay(100);

		if (retry_count > ESP8266_MAX_RETRY_TIME)
		{
			pc_printf("Set single connection model failed\r\n");
			retry_count = 0;
			wifi_config_step--;
			return _FAILED;
		}
	}

	HAL_Delay(100);
	retry_count = 0;


	// Set transparent transmission
	pc_printf("Trying to set transparent transmission mode\r\n");
	wifi_config_step++;

	while (esp8266_TransmitCmd((uint8_t*) "AT+CIPMODE=1", OK_ACK, 1000, WITH_NEWLINE) != _SUCCEED)
	{
		retry_count++;
		HAL_Delay(100);

		if (retry_count > ESP8266_MAX_RETRY_TIME)
		{
			pc_printf("Set transparent transmission mode failed\r\n");
			retry_count = 0;
			wifi_config_step--;
			return _FAILED;
		}
	}

	HAL_Delay(100);
	retry_count = 0;


	// Connect to TCP server
	pc_printf("Trying to connect TCP server\r\n");
	wifi_config_step++;

	while (esp8266_ConnectServer() != _CONNECTED)
	{
		retry_count++;
		HAL_Delay(100);
		if (retry_count > ESP8266_MAX_RETRY_TIME)
		{
			pc_printf("Connect TCP server failed\r\n");
			retry_count = 0;
			wifi_config_step--;
			return _FAILED;
		}
	}

	HAL_Delay(100);
	retry_count = 0;


	// Enable data send (transparent transmission)
	pc_printf("Trying to enable data send\r\n");
	wifi_config_step++;

	while (esp8266_TransmitCmd((uint8_t*) "AT+CIPSEND", OK_ACK, 1000, WITH_NEWLINE) != _SUCCEED)
	{
		retry_count++;
		HAL_Delay(100);
		if (retry_count > ESP8266_MAX_RETRY_TIME)
		{
			pc_printf("Set transparent transmission mode failed\r\n");
			retry_count = 0;
			wifi_config_step--;
			return _FAILED;
		}
	}

	trans_state = _TRANS_ENBALE;
	HAL_Delay(100);
	retry_count = 0;

	return _SUCCEED;
}
