#ifndef _Mqtt_client_h_
#define _Mqtt_client_h_

#define R_LED 4
#define G_LED 16
#define B_LED 17

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include "configuration.h"
//debug
#include <preferences.h>

//Preferences preferences;

struct subscribtion
{
  void(*callback_function)(String);
  String topic;
};

struct TopicMsg
{
  String topic;
  String msg;
};

enum : byte {
  WLAN_DOWN_MQTT_DOWN,
  WLAN_STARTING_MQTT_DOWN,
  WLAN_UP_MQTT_DOWN,
  WLAN_UP_MQTT_STARTING,
  WLAN_UP_MQTT_UP
} connectionState;

void add_subscription(void(*callback_function)(String), String topic);
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttSubscribe(uint16_t packetId, uint8_t qos);
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
void onMqttPublish(uint16_t packetId);
void setup_wifi();
void setup_mqtt();
void check_connection(unsigned long time_between_checks);
void publish(String topic, String payload);
void subscribe(String topic);
String ip2string(const IPAddress& ipAddress);
void set_led(int color);
void connect_wlan_mqtt();
void connection_setup();
void connection_handler();
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

#endif