/**
  ************************************************************************************************
  * @file           : mqttclient.c
  * @brief          : This file contains functions for MQTT client connection and communication
  ************************************************************************************************
*/


// Includes
#include <stdlib.h>
#include <mqttclient.h>
#include <MQTTConnect.h>
#include <MQTTPacket.h>
#include <transport.h>
#include <net_conf.h>
#include "uart_com.h"
#include "main.h"


// Global variables
int mqtt_transport_publishGetData(uint8_t *buf, int buflen);
uint32_t mqtt_msgId = 0;
uint8_t mqtt_PacketBuf[MQTT_PacketBuffSize];

int mqtt_buflen = MQTT_PacketBuffSize;
int mqtt_serialLen = 0;
char responMsg = -1;
int packageID = 0;
int packagePos;



/**
  * @brief  Function to transmit packet to broker
  * @param buf: Buffer with package
  * @param buflen: Length of buffer
  * @retval Buffer length
  */
int mqtt_transport_sendPacketBuffer(uint8_t *buf, int buflen)
{
	// MQTT Head may have 0x00
	ESP_RecvEndFlag = 0;
	memset(ESP_RxBUF, 0, ESP_MAX_RECVLEN);

	HAL_UART_Transmit(&huart1, buf, buflen, 0xff);

	return buflen;
}


/**
  * @brief  Function to receive packet from broker
  * @param buf: Buffer with received package
  * @param buflen: Length of received buffer
  * @retval Buffer length
  */
int mqtt_transport_getdata(uint8_t *buf, int buflen) {

	if (ESP_RecvEndFlag == 1) {
		memcpy(buf, (const char*) ESP_RxBUF, buflen);

		//pc_printf(&buf);
		ESP_RxLen = 0;
		ESP_RecvEndFlag = 0;
		memset(ESP_RxBUF, 0, ESP_MAX_RECVLEN);
	}

	return buflen;
}


/**
  * @brief  Function to connect to a MQTT broker
  * @retval Connection result, 1 on success
  */
uint8_t mqtt_ConnectServer(void)
{
	uint8_t sessionPresent = 0;
	uint8_t connack_rc = 0;

	MQTTPacket_connectData ConnectData = MQTTPacket_connectData_initializer;
	ConnectData.clientID.cstring = MQTT_CLIENTID;
	ConnectData.username.cstring = MQTT_USERNAME;
	ConnectData.password.cstring = MQTT_PASSWORD;
	ConnectData.keepAliveInterval = MQTT_KeepAliveInterval;
	ConnectData.MQTTVersion = 4;
	ConnectData.cleansession = 1;

	pc_printf("Trying to connect MQTT server\r\n");

	// Use connack_rc as command length here
	mqtt_serialLen = MQTTSerialize_connect(mqtt_PacketBuf, MQTT_PacketBuffSize, &ConnectData); // build connect packet
	mqtt_transport_sendPacketBuffer(mqtt_PacketBuf, mqtt_serialLen);

	do
	{
		if (responMsg != CONNACK && MQTT_RecvEndFlag == 1)
		{
			responMsg = MQTTPacket_read(mqtt_PacketBuf, MQTT_PacketBuffSize, mqtt_transport_getdata);
		}

	} while (MQTTDeserialize_connack(&sessionPresent, &connack_rc, mqtt_PacketBuf, MQTT_PacketBuffSize) != 1 || connack_rc != 0);

	// Enable interrupt again
	HAL_UART_Receive_DMA(&huart1, ESP_RxBUF, ESP_MAX_RECVLEN);

	if (connack_rc != 0)
	{
		pc_printf("connack_rc:%uc\r\n", connack_rc);
	}

	pc_printf("Connect Success!\r\n");

	return 1;
}


/**
  * @brief  Function to send a publish for a topic
  * @param topic: Topic to publish for
  * @param buf: Buffer with data to be published
  * @retval None
  */
void mqtt_TransmitPublish(char *topic, char *buf)
{
	int length;
	//char cntString[3];

	MQTTString TopicName = MQTTString_initializer;
	TopicName.cstring = topic;

	//sprintf(cntString, "%d", cnt);

	// Serialize package and get length
	length = MQTTSerialize_publish(mqtt_PacketBuf, MQTT_PacketBuffSize, 0, 0, 0, 1, TopicName, buf, strlen(buf));

	// Transmit new package to broker
	mqtt_transport_sendPacketBuffer(mqtt_PacketBuf, length);
}
