/**
  ************************************************************************************************
  * @file           : esp8266.h
  * @brief          : Header for esp8266.c file.
  *                   This file contains the common defines and the function and variable exports
  ************************************************************************************************
*/


#ifndef __ESP8266_H
#define __ESP8266_H


#include "main.h"
//#include "net_conf.h"
//#include <string.h>
//#include <stdlib.h>
//#include "usart.h"


// Defines
#define OK_ACK            (uint8_t*)"OK"
#define TRANS_QUIT_CMD    (uint8_t*)"+++"

#define WITH_NEWLINE     1
#define WITHOUT_NEWLINE  0

#define ESP8266_MAX_TIMEOUT     (uint16_t)0x0fff
#define ESP8266_MAX_RETRY_TIME  10


// Typedefs
typedef enum __WIFI_StateTypDef {
	_FAILED = 0,
	_SUCCEED = 1,
	_TIMEOUT = 2,
	_MATCHERROR = 3,    // cmd match error
	_MATCHOK = 4,       // cmd match ok
	_ONLINE = 5,        // wifi connect
	_OFFLINE = 6,       // wifi disconnect
	_CONNECTED = 7,     // server connected
	_DISCONNECTED = 8,  // server disconnected
	_TRANS_ENBALE = 9,
	_TRANS_DISABLE = 10,
	_UNKNOWN_STATE = 0xee,
	_UNKNOWN_ERROR = 0xff
} WIFI_StateTypeDef;

// Function exports
extern WIFI_StateTypeDef esp8266_SetUpTCPConnection(void);


#endif
