#ifndef main_h
#define main_h

#include <Arduino.h>
#include <WiFi.h>
#include <Ethernet.h>
#include <AsyncMqttClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebserver.h>
#include <AsyncElegantOTA.h>

#include "HAL.h"
#include "Mqtt_client.h"
#include "Andon.h"

void setup_ota_server();
void setup();
void loop();

#endif