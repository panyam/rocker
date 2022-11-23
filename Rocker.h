
#ifndef __ROCKER__H__
#define __ROCKER__H__

#define DEBUG
#include "Motor.h"
#include "Wifi.h"
#include "common.h"

class Rocker {
public:
  Rocker();
  void next();

private:
  // This mode if true allows us to set the Wifi details
  // The wifi setup allows us to connect to the rocker via our app and control it
  // Without Wifi setup the rocker can just have its speed adjusted.
  bool inWAPMode;
  WIFI wifi;
  WebServer webserver;
  Motor motor;
};

#endif
