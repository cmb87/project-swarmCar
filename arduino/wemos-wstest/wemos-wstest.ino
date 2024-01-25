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
#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "config.h"

// Timers
unsigned long t0;
unsigned long t1;
unsigned long t2;


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
};

#define SENSORPUBLISHINTERVAL 500

SensorStruct sensorState;

// ======================================================
// Websocket
// ======================================================
// Callback function to handle WebSocket events

void setMotorSpeed(int pinA, int pinB, int speed) {

  int direction = 0;

  if (speed>0) {
    direction = 1;
  } else if (speed<0){
    direction = -1;
  }

  speed = abs(speed);
  
  Serial.print("Motor: ");
  Serial.print(speed);
  Serial.print(" Direction: ");
  Serial.print(direction);
  Serial.println();

}

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

    int leftPWM = constrain(cmds.y + cmds.x/2, -255, 255);
    int rightPWM = constrain(cmds.y - cmds.x/2, -255, 255);

    setMotorSpeed(1, 3, leftPWM);
    setMotorSpeed(2, 4, rightPWM);

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
  doc["r"] = 25.5;
  doc["h"] = 60.0;

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
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("WEMOS D1 IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Wifi Connection ready :)");

  // ---------------- Websocket ----------------
  connectWebSocket();

  // ---------------- Timers ----------------
  t0 = millis();
  t1 = millis();
  t2 = millis();

  // ---------------- Light ----------------
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output

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
  if(videoSocket.available()) {
    videoSocket.poll();
  }

  // State Publish
  if ( millis()-t0 > SENSORPUBLISHINTERVAL ) {
    sendJsonData();
    t0 = millis();
  }

  delay(10);

}


