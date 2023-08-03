#include "Mqtt_client.h"

//debug
#define RED 1
#define GREEN 2
#define BLUE 3
#define ORANGE 4
#define WHITE 5
#define VIOLET 6

AsyncMqttClient mqttClient;
unsigned long last_wifi_check;
bool mqtt_connected;
int connection_fails = 0;

bool blink_bit;
unsigned long last_blink;

std::vector<subscribtion> sub;
std::vector<TopicMsg> to_publish;

// add a topic an corresponding callback function, wich wil be executed when a message arrives
void add_subscription(void(*callback_function)(String), String topic) {
  delay(500);
  if (WiFi.status() == WL_CONNECTED && mqttClient.connected()) mqttClient.subscribe(topic.c_str(), 1);
  sub.push_back({callback_function, topic});
}

// confirm connection was succesfull
void onMqttConnect(bool sessionPresent) {
  Serial.println("[BROKER CONNECTED]");
}

// notify connection to MQTT broker was lost and try to reconnect
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("[DISCONNECTED FROM BROKER]");
  Serial.println("RECONNECTING...");
  if (WiFi.status() == WL_CONNECTED) mqttClient.connect();
}

// confirm subscribtion was succesfull
void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.print("[SUBSCRIBE ACKNOWLEDGED]");
  Serial.print(" packetId: ");
  Serial.print(packetId);
  Serial.print(" qos: ");
  Serial.println(qos);
}

// show data of incomming message an handle callback function for the subscribtions
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {

  Serial.println("[MESSAGE RECEIVED]");
  Serial.print("  topic: ");
  Serial.print(topic);
  Serial.print("  feedbackTopic: ");
  Serial.print("{topic}");
  Serial.print("  qos: ");
  Serial.print(properties.qos);
  Serial.print("  dup: ");
  Serial.print(properties.dup);
  Serial.print("  retain: ");
  Serial.print(properties.retain);
  Serial.print("  len: ");
  Serial.print(len);
  Serial.print("  index: ");
  Serial.print(index);
  Serial.print("  payload: ");
  Serial.println(payload);

  // Handle incomming messages
  for (auto s = sub.begin(); s != sub.end(); ++s) {
    if (strcmp(topic, s->topic.c_str()) == 0) {
      Serial.println("Topic match");
      s->callback_function(String(payload));
    }
  }
}

// confirm successfull publish
void onMqttPublish(uint16_t packetId) {
  Serial.print("** Published");
  Serial.print("  packetId: ");
  Serial.print(packetId);
  Serial.print("\n\n");
}

// setup wifi and mqtt connection
void connection_setup() {
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setKeepAlive(MQTT_KEEP_ALIVE).setCleanSession(false).setWill(MQTT_LASTWILL_TOPIC, 2, true, "offline").setCredentials("username", "password").setClientId("m");
}

unsigned long last_wifi_connection = 0;

// handle wifi and mqtt connection and reconnect
void connection_handler() {
  switch (connectionState)
  {
  case WLAN_DOWN_MQTT_DOWN:
    Serial.println();
    Serial.print("CONNECTING TO ");
    Serial.println(SSID);

    WiFi.begin(SSID, PASSWORD);
    connectionState = WLAN_STARTING_MQTT_DOWN;
    break;
  
  case WLAN_STARTING_MQTT_DOWN:
    if (WiFi.status() == WL_CONNECTED) {
      connectionState = WLAN_UP_MQTT_DOWN;
      delay(10);
    } else {
      delay(10);
    }
    break;

  case WLAN_UP_MQTT_DOWN:
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connecting to MQTT...");
      mqttClient.connect();
      connectionState = WLAN_UP_MQTT_STARTING;
    } else {
      Serial.println("WiFi Connection lost -> Reconnecting...");
      connectionState = WLAN_DOWN_MQTT_DOWN;
    }
    break;

  case WLAN_UP_MQTT_STARTING:
    if (mqttClient.connected()) {
      connectionState = WLAN_UP_MQTT_UP;
      Serial.println("MQTT Connected");
      String online_msg = ip2string(WiFi.localIP()) + " - online";
      mqttClient.publish(MQTT_LASTWILL_TOPIC, 1, false, online_msg.c_str());

      // (re)subscribe MQTT topics
      if (!sub.empty()) {
        for (auto s = sub.begin(); s != sub.end(); ++s) {
          subscribe(s->topic);
        }
      }

      // publish all messages from buffer 
      if (!to_publish.empty()) {
        for (auto p = to_publish.begin(); p != to_publish.end(); ++p) {
          publish(p->topic, p->msg);
          delay(1000);
        }
        to_publish.clear();
      }
    }

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("MQTT reconnect failed -> reconnecting WiFi...");
      mqttClient.connected();
      connectionState = WLAN_DOWN_MQTT_DOWN;
    }
    break;

  case WLAN_UP_MQTT_UP:
    last_wifi_connection = millis();

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi Connection lost -> Reconnecting...");
      WiFi.disconnect();
      connectionState = WLAN_DOWN_MQTT_DOWN;
    }

    if (!mqttClient.connected()) {
      Serial.println("MQTT Connection lost -> Reconnecting...");
      mqttClient.disconnect();
      connectionState = WLAN_UP_MQTT_DOWN;
    }

    break;
  
  default:
    connectionState = WLAN_DOWN_MQTT_DOWN;
    break;
  }

  if ((unsigned long)(millis() - last_blink) > 1000) {
    last_blink = millis();
  } else if ((unsigned long)(millis() - last_blink) < 100) {
    blink_bit = true;
  } else {
    blink_bit = false;
  }
  

  if (WiFi.status() != WL_CONNECTED || !mqttClient.connected()) {
    if (blink_bit)set_led(WHITE);
  }

  if (connection_fails > 5) {
    Serial.println("Too many failed connection atempts -> restart device...");
    ESP.restart();
  }

  if ((unsigned long)(millis() - last_wifi_connection) > 60000) {
    Serial.println("1 Minute passed since last successfull connection -> reconnect");
    WiFi.disconnect();
    mqttClient.disconnect();
    connectionState = WLAN_DOWN_MQTT_DOWN;
    last_wifi_connection = millis();
    connection_fails++;
  }
} // connection_handler

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.disconnected.reason);
  //Serial.println("Trying to Reconnect");
  //WiFi.begin(ssid, password);
}

// Setup the mqtt client
void setup_mqtt() {
    int connect_counter = 0;
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onSubscribe(onMqttSubscribe);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.onPublish(onMqttPublish);
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setKeepAlive(MQTT_KEEP_ALIVE).setCleanSession(false).setWill(MQTT_LASTWILL_TOPIC, 2, true, "offline").setCredentials("username", "password").setClientId("m");
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
    // only wait a maximum of 10s
    set_led(WHITE);
    while (!mqttClient.connected() && connect_counter < 1000) {
      delay(10);
      connect_counter++;
    }

    // if the connection was sucessfull send online message an handle communications
    if (mqttClient.connected()) {
      set_led(GREEN);
      String online_msg = ip2string(WiFi.localIP()) + " - online";
      mqttClient.publish(MQTT_LASTWILL_TOPIC, 1, false, online_msg.c_str());

      // resubscribe after connection was lost
      if (!sub.empty()) {
        for (auto s = sub.begin(); s != sub.end(); ++s) {
          subscribe(s->topic);
        }
      }

      // publish all messages from buffer 
      if (!to_publish.empty()) {
        for (auto p = to_publish.begin(); p != to_publish.end(); ++p) {
          publish(p->topic, p->msg);
          delay(1000);
        }
        to_publish.clear();
      }
    }
}

// publish data on topic
void publish(String topic, String payload) {
  // can only publish if MQTT is connected
  if (WiFi.status() == WL_CONNECTED && mqttClient.connected()) {
    Serial.print("[Publish] ");
    Serial.print(topic);
    Serial.print(" ");
    Serial.println(payload);
    mqttClient.publish(topic.c_str(), 1, false, payload.c_str());
  } else {
    // when publish is not possible save msg in buffer
    Serial.println("Cannot publish, add msg to buffer");
    to_publish.push_back({topic, payload});
  }
}

// subscribe to topic
void subscribe(String topic) {
  mqttClient.subscribe(topic.c_str(), 1);
}

// get representation of IP as string
String ip2string(const IPAddress& ipAddress) {
  return String(ipAddress[0]) + String(".") +
        String(ipAddress[1]) + String(".") +
        String(ipAddress[2]) + String(".") +
        String(ipAddress[3]);
}

/*
void log_wifi_state() {
  preferences.begin("wifi_state", false);
  String state = String(static_cast<char>(WiFi.status()));
  String history = preferences.getString("state");
  //String last_state = history[0];
  
  preferences.putString("state", state);
}*/

void set_led(int color) {
  switch (color)
  {
  case RED:
    digitalWrite(R_LED, true); digitalWrite(G_LED, false); digitalWrite(B_LED, false);
    break;
  
  case GREEN:
    digitalWrite(R_LED, false); digitalWrite(G_LED, true); digitalWrite(B_LED, false);
    break;
  
  case BLUE:
    digitalWrite(R_LED, false); digitalWrite(G_LED, false); digitalWrite(B_LED, true);
    break;
  
  case ORANGE:
    digitalWrite(R_LED, true); digitalWrite(G_LED, true); digitalWrite(B_LED,false);
    break;
  
  case WHITE:
    digitalWrite(R_LED, true); digitalWrite(G_LED, true); digitalWrite(B_LED, true);
    break;
  
  case VIOLET:
    digitalWrite(R_LED, true); digitalWrite(G_LED, false); digitalWrite(B_LED, true);
    break;
  
  default:
    digitalWrite(R_LED, false); digitalWrite(G_LED, false); digitalWrite(B_LED, false);
    break;
  }  
} // set_led
