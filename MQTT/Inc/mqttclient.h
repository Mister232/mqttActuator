/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Sergio R. Caprile - "commonalization" from prior samples and/or documentation extension
 *******************************************************************************/

#ifndef __MQTTCLIENT_H
#define __MQTTCLIENT_H

#include <stdio.h>

#define MQTT_KeepAliveInterval   60
#define MQTT_PacketBuffSize      1024

#define MQTT_RecvEndFlag         ESP_RecvEndFlag


extern uint8_t mqtt_ConnectServer(void);
extern uint8_t mqtt_PacketBuf[MQTT_PacketBuffSize];
extern void mqtt_TransmitPublish(char *topic, char *buf);
extern int mqtt_checkAndReceivePublish(unsigned char *outBuf);
extern void mqtt_SubscribeTopic(char *topicName);
extern int mqtt_send_ping(uint32_t waitms);

#endif
