
#ifndef __WIFI_AP_H__
#define __WIFI_AP_H__

#include <WiFiNINA.h>
#include "Arduino.h"

enum WIFIState {
  WIFI_IDLE,
  STARTING_WAP,
  WAP_STARTED,
  CONNECTING_TO_WIFI,
  WIFI_CONNECTED,
};

/**
 * The WAP simply allows us to connect to the rocker for the first time to set the
 * "real" wifi ssid and password and its hosten  it needs to use going forward.
 *
 * Once the ssid, password and hostname are set it can be accessed via any browser normally.
 */
class WIFI {
public:
  WIFI(char *ssid = NULL, char *password = "password", char *hostname = NULL, int keyIndex = 0);
  void configure(String ssid, String password, String hostname, int keyIndex = 0);
  void setup();
  void next();
  void printWAPStatus();
  void printWiFiStatus();
  void printCurrentNet();
  void printMacAddress(byte mac[]);
  bool start(int mode = 0, int numRetries = 5);
  void stop();
  bool handleAPClient();
  bool loadFromEEPROM();
  void saveToEEPROM();
  bool inWAPMode() { return state == WAP_STARTED; }

public:
  String ssid;
  String password;
  String hostname;
  int keyIndex = 0;  // your network key Index number (needed only for WEP)

protected:
  bool tryStartingWAP();
  bool tryConnectingToWifi();

private:
  // Connection status
  int status;

  // What state are we in?
  int state;

  // Which try are we on in the number of retries?
  int currTry;

  // How many times should a AP/WIFI connection be done
  int numTries;

  // Which mode to connect in
  int wifiMode;
};

#endif
