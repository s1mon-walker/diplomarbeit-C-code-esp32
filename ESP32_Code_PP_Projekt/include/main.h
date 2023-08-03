#ifndef main_h
#define main_h

#include <Arduino.h>
#include <WiFi.h>
#include <Ethernet.h>
#include <AsyncMqttClient.h>
#include <Preferences.h>

//#include "HAL.h"
#include "Mqtt_client.h"
#include "PP_cap.h"

void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void setup();
void loop();

#endif