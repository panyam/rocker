#include "Motor.h"
#define DEBUG
#include "common.h"

Motor::Motor(int rpwmPin, int lpwmPin, int ns, int ls, int hs) : rpwmOutputPin(rpwmPin), lpwmOutputPin(lpwmPin), numSpeeds(ns), lowestSpeed(ls), highestSpeed(hs) {
  currSpeed = 0;
}

void Motor::setup() {
  pinMode(rpwmOutputPin, OUTPUT);
  pinMode(lpwmOutputPin, OUTPUT);
  // Start with 0 speed for safety
  // analogWrite(lpwmOutputPin, 0);
  DPRINTLN("Resetting motor to 0");
}

void Motor::setSpeed(int speed) {
  DPRINTLN((String)"Setting speed to: " + speed + ", " + rpwmOutputPin + ", " +  lpwmOutputPin);

  if (speed > numSpeeds) {
    speed = 0;
  } else if (speed < 0) {
    // what do we do here?
    // for now dont go below 0
    speed = 0;
  }

  currSpeed = speed;
  analogWrite(rpwmOutputPin, 0);
  if (currSpeed == 0) {
    // switch it off
    DPRINTLN("Stopping Motor");
    analogWrite(lpwmOutputPin, 0);
  } else {
    DPRINTLN("Current speed: " + (String)currSpeed);
    int incr = (highestSpeed - lowestSpeed) / numSpeeds;
    int currVal = lowestSpeed + (currSpeed - 1) * incr;
    analogWrite(lpwmOutputPin, currVal);
  }
}

void Motor::decreaseSpeed() {
  setSpeed(currSpeed - 1);
}

void Motor::increaseSpeed() {
  setSpeed(currSpeed + 1);
}
