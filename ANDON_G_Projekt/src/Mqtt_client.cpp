#include "Mqtt_client.h"

AsyncMqttClient mqttClient;
unsigned long last_wifi_check;
bool mqtt_connected;
int connection_fails = 0;

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

// Connect to WiFi and print IP Address on serial monitor
void setup_wifi() {
  unsigned long timer = millis();
  
  delay(10);
  Serial.println();
  Serial.print("CONNECTING TO ");
  Serial.println(SSID);

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);

  // only wait a maximum of 10 seconds on connection
  while ((WiFi.status() != WL_CONNECTED) && ((unsigned long)(millis() - timer) < 10000)) {
    delay(500);
    Serial.print(".");
  }

  // if the connection was succesfull, print IP and setup MQTT
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi CONNECTED");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    setup_mqtt();

  } else {
    Serial.println("connection failed, retry later");
    last_wifi_check = millis();
  }
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
    while (!mqttClient.connected() && connect_counter < 1000) {
      delay(10);
      connect_counter++;
    }

    // if the connection was sucessfull send online message an handle communications
    if (mqttClient.connected()) {
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

// checking the connection to WiFi and MQTT periodicly
void check_connection(unsigned long time_between_checks) {
  // only performing the connection check every X seconds
  if ((unsigned long)(millis() - last_wifi_check) > (time_between_checks * 1000)) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi connection lost, reconnecting...");
      unsigned long timer = millis();
      bool indication_blink = true;

      WiFi.reconnect();

      // wait a maximum of 10 seconds for reconnect and indicate connection troubles with led
      while ((WiFi.status() != WL_CONNECTED) && ((unsigned long)(millis() - timer) < 10000)) {
        delay(500);
        Serial.print(".");
        indication_blink = !indication_blink;
      }

      // if connection was succesfull setup MQTT again
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi CONNECTED");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());

        setup_mqtt();

      } else {
        Serial.println("connection failed, retry later");
        connection_fails++;
      }

      // restart the device after too many failed connections
      if (connection_fails > MAX_CONNECTION_FAILS) ESP.restart();
      
      last_wifi_check = millis();
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
