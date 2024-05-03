#ifndef MQTT_MANAGER
#define MQTT_MANAGER

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// 브로커 주소
const char* mqtt_server = "nextit.or.kr";
// 포트 번호
const int mqtt_port = 21883;
// 메세지 버퍼 크기
const int MSG_BUFFER_SIZE = 200;

WiFiClient wifiClient;              // MQTT 가 WiFi 사용할 수 있게 세팅
PubSubClient pubClient(wifiClient); // MQTT 본체

// MQTT 설정
void setupMQTT();
void receivedMQTTCallback(char* topic, byte* payload, unsigned int length);

void setupMQTT() {
  pubClient.setServer(mqtt_server, mqtt_port);
  pubClient.setCallback(receivedMQTTCallback);
}

// 자동 재연결
void reconnect() {
  while(!pubClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "Ssamssara";
    clientId += String(random(0xffff), HEX);

    if(pubClient.connect(clientId.c_str())) {
      Serial.println("connected");
      pubClient.publish("Ssamssara", "Smart Farm");
      pubClient.subscribe("Ssamssara");
    }else{
      Serial.print("failed, rec=");
      Serial.print(pubClient.state());
      Serial.println(" try again in 3 seconds");
      delay(3000);
    }
  }
}

// MQTT 로 Message 발송
void publish(String topicStr, String payloadStr) {
  int topicLength = topicStr.length() + 1;
  int payloadLength = payloadStr.length() +1;

  char *topic = new char[topicLength + 2 ];
  char *payload = new char[payloadLength + 2];

  topicStr.toCharArray(topic, topicLength);
  payloadStr.toCharArray(payload, payloadLength);

  pubClient.loop();
  pubClient.publish(topic, payload); // Server 로 JSON data 전송

  String log = String("publish topic : " + topicStr + " msg : " +payloadStr);
  Serial.println(log);

  delete topic;
  delete payload;
}

// Server 로부터 Message 수신 + 원격제어
void receivedMQTTCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  for(int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

#endif