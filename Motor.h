#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "Arduino.h"

class Motor {
public:
  Motor(int rpwmPin, int lpwmPin, int ns = 5, int ls = 180, int hs = 320);
  void setup();
  void setSpeed(int speed);
  void decreaseSpeed();
  void increaseSpeed();
  int currSpeed;
  int numSpeeds;
  int lowestSpeed;
  int highestSpeed;
  int rpwmOutputPin;
  int lpwmOutputPin;
};

#endif
