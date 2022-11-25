
#include "Web.h"

WebServer::WebServer(int port) : server(port) {
}

void WebServer::next() {
  WiFiClient client = server.available();   // listen for incoming clients
  if (!client) {                             // if you get a client,
    return;
  }
  Serial.println("new client");           // print a message out the serial port
  int lineNum = 0;
  String reqLine = "";
  String currentLine = "";                // make a String to hold incoming data from the client
  void *reqctx;
  while (client.connected()) {            // loop while the client's connected
    if (client.available()) {             // if there's bytes to read from the client,
      char c = client.read();             // read a byte, then
      // Serial.write(c);                    // print it out the serial monitor
      if (c != '\n' && c != '\r') {    // if you got anything else but a carriage return character,
        currentLine += c;      // add it to the end of the currentLine
      } else if (c == '\n') {
        if (currentLine.length() == 0) {
          // request headers has ended so can be processed
          onBodyStarted(client, reqctx);
          break;
        } else {
          if (lineNum == 0) {
            reqLine = currentLine;
            int sp = reqLine.indexOf(" ");
            int sp2 = reqLine.lastIndexOf(" ");
            String method = reqLine.substring(0, sp);
            String path = reqLine.substring(sp, sp2);
            String version = reqLine.substring(sp2);
            method.trim();
            path.trim();
            version.trim();
            if (onRequest) reqctx = onRequest(method, path, version);
          }
          currentLine = "";
        }
        lineNum++;
      }
    }
  }
  // close the connection:
  client.stop();
  Serial.println("client disconnected");
}
