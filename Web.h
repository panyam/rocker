
#ifndef __WEB_SERVER_H__
#define __WEB_SERVER_H__

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
  void next();

public:
  void *(*onRequest)(String method, String path, String version);
  void (*onHeader)(void *reqctx, String method, String path, String version);
  void (*onBodyStarted)(WiFiClient &client, void *reqctx);

private:
  WiFiServer server;
};

#endif
