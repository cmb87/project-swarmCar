/*
https://wokwi.com/projects/360194707275211777
https://github.com/gilmaimon/ArduinoWebsockets/issues/101
https://stackoverflow.com/questions/64175514/esp32-cam-websocket-wifimulti-reconnect
https://how2electronics.com/connecting-esp32-to-amazon-aws-iot-core-using-mqtt/
https://randomnerdtutorials.com/esp32-cam-ov2640-camera-settings/

// Websockets...
// I finallyfound the answer thanks to this comment. I was using the first certificate, but the second was needed. Sorry, my bad.
// The right command is:
// $ openssl s_client -showcerts -connect websocket.org:443
openssl s_client -showcerts -connect mydomain.cloud:443 ==> use the second displayed certificate for wss!

Note: When using MQTT and Websockets together without a sufficient delay the WSS server seems to cause a problem. Either the image is to big or the delay????

*/

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#include <QMC5883LCompass.h>
#include <NewPing.h>
#include <ESP8266WiFiMulti.h>
#include "config.h"



ESP8266WiFiMulti wifiMulti;

// Websocket
using namespace websockets;
WebsocketsClient webSocket;
WebsocketsClient videoSocket;

struct CommandoStruct {
  long t;
  long x;
  long y;
};

struct SensorStruct {
  float r;
  float h;
  float rssi;
  float bat;
};

#define SENSORPUBLISHINTERVAL 500

SensorStruct sensorState;

// Motor
#define enA D8
#define in1 D6
#define in2 D5
#define in3 D0
#define in4 D2
#define enB D7

// ------------------ MAG ------------------
/* Assign a unique ID to this sensor at the same time */
//QMC5883LCompass compass;


// ------------------ HC-SR04 Ultrasonic Sensor  ------------------
/* Assign a unique ID to this sensor at the same time */
#define trigPin D4
#define echoPin D3
#define ultraSonicInterval 1000
#define MAX_DISTANCE 300 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(trigPin, echoPin, MAX_DISTANCE); // NewPing setup of pins and maximum distance.


// defines variables
//long duration;
long distance;
long currentTimeUltrasonic = 0;

// Timers
unsigned long t0;
unsigned long t1;
unsigned long t2;

// VoltageDivider for Battery
// https://randomnerdtutorials.com/esp8266-adc-reading-analog-values-with-nodemcu/
// IMPORTANT NOTE: The voltage range of ADC in ESP8266 SoC is 0V to 1V. If you want to use the ADC for 0V to 3.3V, 
// then you have to use a voltage divider circuit as shown above. ESP-01 users, who don’t have access to ADC Pin can 
// solder a wire to Pin 6 of ESP8266EX SoC and use it with the voltage divider.

// WARNING: Do not provide more than 1V directly to the ADC Pin (TOUT – Pin 6) of ESP8266EX SoC.

#define analogInPin A0  // ESP8266 Analog Pin ADC0 = A0

// ======================================================
// Motors
// ======================================================

void setMotorSpeed(int pinA, int pinB, int pinEna, int speed) {

  if (speed>0) {
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, HIGH);
    analogWrite(pinEna, abs(speed)); // Send PWM signal to motor A
  } else if (speed<0){
    digitalWrite(pinA, HIGH);
    digitalWrite(pinB, LOW);
    analogWrite(pinEna, abs(speed)); // Send PWM signal to motor A
  } else {
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, LOW);
  }

}

// ======================================================
// Ultrasonic
// ======================================================
void measureDistance() {
  // // Clears the trigPin
  // digitalWrite(trigPin, LOW);
  // delayMicroseconds(2);
  // // Sets the trigPin on HIGH state for 10 micro seconds
  // digitalWrite(trigPin, HIGH);
  // delayMicroseconds(10);
  // digitalWrite(trigPin, LOW);
  // // Reads the echoPin, returns the sound wave travel time in microseconds
  // duration = pulseIn(echoPin, HIGH);
  // // Calculating the distance
  // distance = duration * 0.034 / 2;
  // // Prints the distance on the Serial Monitor
  // Serial.print("Distance: ");
  // Serial.println(distance);
  delay(30);                      // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
  unsigned int uS = sonar.ping(); // Send ping, get ping time in microseconds (uS).
  Serial.print("Ping: ");
  Serial.print(sonar.convert_cm(uS)); // Convert ping time to distance and print result (0 = outside set distance range, no ping echo)
  Serial.println("cm");
  distance = sonar.convert_cm(uS);

}

// ======================================================
// Websocket
// ======================================================
// Callback function to handle WebSocket events

void onMessageCallback(WebsocketsMessage message) {
  // Handle WebSocket messages received from the server
  // Parse and deserialize JSON using ArduinoJson
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message.data());

  if (error) {
    Serial.print("JSON deserialization failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Extract data from JSON
  CommandoStruct cmds;
  cmds.t = doc["t"].as<long>();
  cmds.x = doc["x"].as<long>();
  cmds.y = doc["y"].as<long>();

  // Parse the message data into the struct
  Serial.print("t: ");
  Serial.print(cmds.t);
  Serial.print(" x:");
  Serial.print(cmds.x);
  Serial.print(" y:");
  Serial.print(cmds.y);
  Serial.println();

  if (cmds.t == 0) {

    int leftPWM = constrain(cmds.y - cmds.x/2, -255, 255);
    int rightPWM = constrain(cmds.y + cmds.x/2, -255, 255);

    setMotorSpeed(in2, in1, enA ,leftPWM);
    setMotorSpeed(in4, in3, enB, rightPWM);

  } 
  else if (cmds.t == 1) {
    digitalWrite(BUILTIN_LED, HIGH); 
    Serial.println("Lights on");
  }
  else if (cmds.t == 2) {
    digitalWrite(BUILTIN_LED, LOW); 
    Serial.println("Lights off");
  }
}


void onEventsCallback(WebsocketsEvent event, String data) {
    if(event == WebsocketsEvent::ConnectionOpened) {
        Serial.println("Connnection Opened");
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("Connnection Closed");
    } else if(event == WebsocketsEvent::GotPing) {
        Serial.println("Got a Ping!");
    } else if(event == WebsocketsEvent::GotPong) {
        Serial.println("Got a Pong!");
    }
}


void connectWebSocket() {

  Serial.print("Connecting to ");
  Serial.println(WEBSOCKET_SERVER);

  // Set up the WebSocket client
  webSocket.onEvent(onEventsCallback);
  webSocket.onMessage(onMessageCallback);

  if (webSocket.connect(WEBSOCKET_SERVER)) {
    Serial.println("WebSocket connected!");
  } else {
    Serial.println("WebSocket connection failed!");
  }

  // Video Socket
  // Set up the Video WebSocket client
  Serial.print("Connecting to ");
  Serial.println(WEBSOCKETVIDEO_SERVER);

  videoSocket.onEvent(onEventsCallback);
  if (videoSocket.connect(WEBSOCKETVIDEO_SERVER)) {
    Serial.println("Video WebSocket connected!");
  } else {
    Serial.println("Video WebSocket connection failed!");
  }

  

}

void sendJsonData() {
  // Create a JSON document using ArduinoJson
  StaticJsonDocument<256> doc;
  doc["r"] = sensorState.r;
  doc["h"] = sensorState.h;
  doc["rssi"] = sensorState.rssi;
  doc["bat"] = sensorState.bat;
  // Convert the JSON document to a string
  String jsonString;
  serializeJson(doc, jsonString);

  // Send the JSON string over the WebSocket
  webSocket.send(jsonString);
}


// ======================================================
// Setup
// ======================================================
void setup() {

  Serial.begin(115200);

  // ---------------- Wifi ----------------
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID1, WIFI_PASSWORD1);
  wifiMulti.addAP(WIFI_SSID2, WIFI_PASSWORD2);
  wifiMulti.addAP(WIFI_SSID3, WIFI_PASSWORD3);


  Serial.println("Connecting to Wi-Fi");
 
  // Connect to Wi-Fi using wifiMulti (connects to the SSID with strongest connection)
  Serial.println("Connecting Wifi...");
  if(wifiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  // ---------------- Websocket ----------------
  connectWebSocket();

  // ---------------- Timers ----------------
  t0 = millis();
  t1 = millis();
  t2 = millis();

  // ---------------- Light ----------------
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output


  // ------------------ HC-SR04 Ultrasonic Sensor  ------------------
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  currentTimeUltrasonic = millis();

  // ------------------ MAG ------------------

  //compass.init();
  //compass.setCalibration(-1700, 840, 0, 3056, -2957, 0);
  Serial.println("Compass initialized");
  
  // ---------------- Motors ----------------
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);


  Serial.println("System ready :)");

}


// ======================================================
// Loop
// ======================================================
void loop() {

  // let the websockets client check for incoming messages
  if(webSocket.available()) {
    webSocket.poll();
  }

  // Video Socket
  //if(videoSocket.available()) {
  //  videoSocket.poll();
  // }

  // State Publish
  if ( millis()-t0 > SENSORPUBLISHINTERVAL ) {

    measureDistance();

    sensorState.r = distance;
    sensorState.h = 90;
    sensorState.rssi = WiFi.RSSI();
    sensorState.bat = analogRead(analogInPin);

    sendJsonData();
    t0 = millis();
  }


  delay(10);

}


