
#ifndef __WIFI_AP_H__
#define __WIFI_AP_H__

#include <WiFiNINA.h>
#include "Arduino.h"

class WebServer {
public:
  WebServer(int port = 80);
  void start() {
    server.begin();
  }
  void stop() {
    // server.stop();
  }
  void handleClient();

public:
  void *(*onRequest)(String method, String path, String version);
  void (*onHeader)(void *reqctx, String method, String path, String version);
  void (*onBodyStarted)(WiFiClient &client, void *reqctx);

private:
  WiFiServer server;
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
  void printWAPStatus();
  void printWiFiStatus();
  void printCurrentNet();
  void printMacAddress(byte mac[]);
  void start(int mode = 0);
  void stop();
  bool handleAPClient();
  bool loadFromEEPROM();
  void saveToEEPROM();

  String ssid;
  String password;
  String hostname;
  int keyIndex = 0;  // your network key Index number (needed only for WEP)

private:
  int status;        // connection status
};

#endif
