#include <QMC5883LCompass.h>
#include <Wire.h>




// ------------------ MAG ------------------
/* Assign a unique ID to this sensor at the same time */
QMC5883LCompass compass;


// ------------------ HC-SR04 Ultrasonic Sensor  ------------------
/* Assign a unique ID to this sensor at the same time */
#define trigPin D3
#define echoPin D4
// defines variables
long duration;
int distance;

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


void setup(){
  // ------------------ Serial ------------------
  Wire.begin();


  Serial.begin(115200);
  //myTransfer.begin(Serial);
  Serial.println("Beginning");

  // ------------------ HC-SR04 Ultrasonic Sensor  ------------------
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

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


  delay(3000);

}

// ===================================================================
void moveMotor(int dirPin1, int dirPin2, int enaPin, float u, int speedMin=70, int speedThres=10) {
  float speed = fabs(u); 

  if (speed > 255 ) {
    speed = 255;
  }


  speed = map(speed, 0, 255, 70, 255);


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
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);
}

// ===================================================================
void loop(){

  // Record the time
  //currentMillis = millis();

  // Control Motor
  moveMotor(in1, in2, enA,  ur);
  moveMotor(in3, in4, enB,  ul);


}


