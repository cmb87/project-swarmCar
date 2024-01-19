#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <QMC5883LCompass.h>
#include <Wire.h>
#include <NewPing.h>
#include "config.h"

// ------------------ UDP ------------------
WiFiUDP udp;

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

// ------------------ L298N ------------------
// PWM Wemos D1
#define enA D8
#define in1 D6
#define in2 D5
#define in3 D0
#define in4 A0
#define enB D7


float ur = 0.0;
float ul = 0.0;


// ------------------ Structs ------------------
// Define the struct
struct SensorData {
  long  distance;
  float heading;
};

struct CommandData {
  long motorRightPwm;
  long motorLeftPwm;
};


CommandData receivedData;

// ===================================================================
void setup(){
  // ------------------ Serial ------------------
  //Wire.begin();


  Serial.begin(115200);
  //myTransfer.begin(Serial);
  Serial.println("Beginning");

  // ------------------ HC-SR04 Ultrasonic Sensor  ------------------
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  currentTimeUltrasonic = millis();

  // ------------------ MAG ------------------

  //compass.init();
  //compass.setCalibration(-1700, 840, 0, 3056, -2957, 0);
  Serial.println("Compass initialized");
  
  // ------------------ L298N ------------------
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

    // ------------------ Wifi and UDP ------------------
  // Connect to Wi-Fi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);

  delay(3000);

}

// ===================================================================
void connectWifi(){

} 

// ===================================================================
void moveMotor(int dirPin1, int dirPin2, int enaPin, float u, int speedMin=70, int speedThres=10) {
  
  float speed = fabs(u); 

  if (speed > 255 ) {
    speed = 255;
  }

  speed = map(speed, 0, 255, 0, 255);

  if (u<0){
    digitalWrite(dirPin1, LOW);
    digitalWrite(dirPin2, HIGH);
  } else {
    digitalWrite(dirPin1, HIGH);
    digitalWrite(dirPin2, LOW);
  }
  analogWrite(enaPin, speed); // Send PWM signal to motor A

}

// ===================================================================
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

// ===================================================================
void loop(){

  // ---------------------------------
  // Get distance measurement
  if (currentTimeUltrasonic-millis() > ultraSonicInterval) {
    measureDistance();
    currentTimeUltrasonic = millis();
  }
  
  // ---------------------------------
  // Receive the struct over UDP
  int packetSize = udp.parsePacket();
  if (packetSize) {

    udp.read((uint8_t*)&receivedData, sizeof(receivedData));

    Serial.print("Received motorRightPwm: ");
    Serial.println(receivedData.motorRightPwm);
    Serial.print("Received motorLeftPwm: ");
    Serial.println(receivedData.motorLeftPwm);


    SensorData dataToSend;
    dataToSend.distance = distance;
    dataToSend.heading = 60.0;

    // Respond to the sender if needed
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write((uint8_t*)&dataToSend, sizeof(dataToSend));
    udp.endPacket();

  }

  // ---------------------------------
  // Control Motor
  moveMotor(in1, in2, enA,  receivedData.motorRightPwm);
  moveMotor(in3, in4, enB,  receivedData.motorLeftPwm);

}


