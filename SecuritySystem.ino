/**
 * Security System designed by Austin Zhang
 * www.austinmzhang.com
 * 
 * TODO: Use statistics to store the likelihood of discrepency for a specific area
 * FIX Indexing function
 * Possible memory leak?? Need to investigate.
 */

#include <Wire.h>
#include <LIDARLite.h>
#include"pitches.h"
#include"Servo.h"


// Globals
Servo servoOscillator;
int incrementAngle = 1;
int currentAngle = 24;
int maxAngle = 118;
int minAngle = 24;
int firstRun = true;

LIDARLite lidarLite;
int cal_cnt = 0;
int trig_cnt = 0;

int angleMeasurements[240];//[(100 - 0) / 2 + 2] = {};
int measurementCount[240];//[(100 - 0) / 2 + 2] = {};


void setup()
{
  Serial.begin(9600); // Initialize serial connection to display distance readings

  lidarLite.begin(0, true); // Set configuration to default and I2C to 400 kHz
  lidarLite.configure(0); // Change this number to try out alternate configurations
  servoOscillator.attach(9);
}

void loop()
{

  int currentIndex = currentAngle;//(currentAngle - minAngle) / incrementAngle;

  if (incrementAngle < 0) {
    currentIndex = currentAngle + 120;
  }

  //Lidar Measurement

  int dist;

  // At the beginning of every 100 readings,
  // take a measurement with receiver bias correction
  if ( cal_cnt == 0 ) {
    dist = lidarLite.distance();      // With bias correction
  } else {
    dist = lidarLite.distance(false); // Without bias correction
  }

  // Increment reading counter
  cal_cnt++;
  cal_cnt = cal_cnt % 100;

  // Display distance
  //Serial.print(dist);
  //Serial.println(" cm");


  //Alarm Handling

  if (measurementCount[currentIndex] >= 10 && dist < angleMeasurements[currentIndex] - 30) {
    Serial.print("discrepency ");
    Serial.print(angleMeasurements[currentIndex]);
    Serial.print(" ");
    Serial.println(dist);
    Serial.print("angle");
    Serial.println(currentAngle);
    if (trig_cnt > 0) {
      playAlarm();
      trig_cnt = 0;
    }
    else {
      trig_cnt++;
    }
  }
  else {
    trig_cnt = 0;
  }


  //Servo Handling

  servoOscillator.write(currentAngle);

  currentAngle = currentAngle + incrementAngle;


  if (currentAngle >= maxAngle || currentAngle <= minAngle) { //Max and Min angles
    incrementAngle = -incrementAngle;
    firstRun = false;
  }

  if (!firstRun) {
    if (measurementCount[currentIndex] < 10) {
      angleMeasurements[currentIndex] = (angleMeasurements[currentIndex] * measurementCount[currentIndex] + dist) / (measurementCount[currentIndex] + 1);
      measurementCount[currentIndex] = measurementCount[currentIndex] + 1;
    }
  }

  delay(5);
}

void playAlarm() {
  long timer = millis();
  bool playF = true;

  Serial.println("Alarm Triggered");

  while (abs(millis() - timer) < 2000) {
    int note = NOTE_D5;

    if (playF) {
      note = NOTE_F5;
    }

    tone(3, note, 2 * 90 * .95);

    delay(2 * 90);

    playF = !playF;

    noTone(3);
  }
}
