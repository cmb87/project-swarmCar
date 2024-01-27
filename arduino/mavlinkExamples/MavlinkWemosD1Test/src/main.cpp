#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include <common/mavlink.h>

#include "conf.h"
#include "creds.h"
#include "func.h"


void setup() {

  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(BUILTIN_LED, LOW);
  Serial.begin(MAVLINK_BAUD);

  init_wifi();
}

void loop() {
  unsigned long currentMillis = millis();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("\nReconnecting to ");

    digitalWrite(BUILTIN_LED, HIGH);

    WiFi.disconnect();
    WiFi.reconnect();
  }

  if (currentMillis - previousHeartbeatMillis >= heartbeat_interval) {
    previousHeartbeatMillis = currentMillis;

    send_heartbeat();
    digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
  }

  if (currentMillis - previousSensorSendMillis >= sensorsend_interval) {
    previousSensorSendMillis = currentMillis;

    send_systemstatus();
    send_radiostatus();
    send_distance();
    send_position();
  }

  

  // if (currentMillis - previousTimeoutMillis >= timeout_interval) {
  //   previousTimeoutMillis = millis();
  //   digitalWrite(BUILTIN_LED, HIGH);
  // }

  while (Serial.available() > 0) {
    uint8_t serial_byte = Serial.read();
    parse_mavlink(serial_byte);
  }

  int packetSize = mavlink_udp.parsePacket();
  if (packetSize) {
    uint8_t packetBuffer[MAVLINK_MAX_PACKET_LEN];
    mavlink_udp.read(packetBuffer, MAVLINK_MAX_PACKET_LEN);

    for (int16_t i = 0; i < packetSize; i++) {
      parse_mavlink(packetBuffer[i]);
    }
  }
}
