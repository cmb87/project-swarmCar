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
#include "config.h"

// Timers
unsigned long t0;
unsigned long t1;
unsigned long t2;


// Websocket
using namespace websockets;
WebsocketsClient webSocket;


struct CommandoStruct {
  long type;
  long throttle;
  long turn;
};


// ======================================================
// Websocket
// ======================================================
// Callback function to handle WebSocket events

void onMessageCallback(WebsocketsMessage message) {
  // Handle WebSocket messages received from the server
  Serial.println("Received data from server:");
  Serial.println(message.data());

  // Parse the message data into the struct
  if (message.length() == sizeof(CommandoStruct)) {
    CommandoStruct receivedData;
    memcpy(&receivedData, message.data(), sizeof(CommandoStruct));

    if (receivedData.type == 0) {

      int leftSpeed, rightSpeed;  
      //getMotorSpeedsFromJoystick(receivedData.turn, receivedData.throttle, leftSpeed, rightSpeed);  

      // Forward
      if ( receivedData.throttle >= 0.0) {

        Serial.print("BWD: ");
        Serial.print(leftSpeed);
        Serial.print(", ");
        Serial.print(rightSpeed);
        Serial.println();

      // Backward
      } else {
        Serial.print("FWD: ");
        Serial.print(leftSpeed);
        Serial.print(", ");
        Serial.print(rightSpeed);
        Serial.println();

      }
    } 
    else if (receivedData.type == 1) {
      digitalWrite(BUILTIN_LED, HIGH); 
      Serial.println("Lights on");
    }
    else if (receivedData.type == 2) {
      digitalWrite(BUILTIN_LED, LOW); 
      Serial.println("Lights off");
    }

  } else {
    Serial.println("Invalid message length for CommandoStruct");
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

  Serial.print("Connecting to ");
  Serial.println(WEBSOCKET_SERVER);

  // Set up the WebSocket client
  webSocket.onEvent(onEventsCallback);
  webSocket.onMessage(onMessageCallback);
  webSocket.connect(WEBSOCKET_SERVER);

  Serial.println("Websocket ready :)");

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

  delay(10);

}


