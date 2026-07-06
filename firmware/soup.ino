#include <ESP32Servo.h>
#include <Wire.h>

int servopin1 = 8;
int servopin2 = 20;
int servopin3 = 21;




int randnum = 0;
bool servoturning = false;


Servo servo1;
Servo servo2;
Servo servo3;
bool play = true;
int pos = 0;
int pos2 = 0;
int pos3 = 0;

void setup() {

ESP32PWM::allocateTimer(0);
ESP32PWM::allocateTimer(1);
ESP32PWM::allocateTimer(2);
ESP32PWM::allocateTimer(3);
servo1.setPeriodHertz(50);
servo2.setPeriodHertz(50);
servo3.setPeriodHertz(50);


servo1.attach(servopin1, 500, 2500);
servo2.attach(servopin2, 500, 2500);
servo3.attach(servopin3, 500, 2500);

}

void loop() {
  if (play) {

    randnum = random(1, 4);

    if (randnum == 1) {
      for (pos = 135; pos >= 45; pos--) {
        servo1.write(pos);
        delay(15);
      }
      delay(300);
      for (pos = 45; pos <= 135; pos++) {
        servo1.write(pos);
        delay(15);
      }
    } else if (randnum == 2) {
      for (pos2 = 90; pos2 <= 180; pos2++) {
        servo2.write(pos2);
        delay(15);
      }
      delay(300);
      for (pos2 = 180; pos2 >= 90; pos2--) {
        servo2.write(pos2);
        delay(15);
      }
    } else if (randnum == 3) {
      for (pos3 = 0; pos3 <= 90; pos3++) {
        servo3.write(pos3);
        delay(15);
      }
      delay(300);
      for (pos3 = 90; pos3 >= 0; pos3--) {
        servo3.write(pos3);
        delay(15);
      }
    }
  }
}