#define DEBUG   //If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.
#include "common.h"
#include "Button.h"
#include "Motor.h"
#include "Wifi.h"

typedef struct Request {
  String method;
  String path;
} Request;

void whiteButtonHandler(Button *b, int event, unsigned long currTime);
void blueButtonHandler(Button *b, int event, unsigned long currTime);
void yellowButtonHandler(Button *b, int event, unsigned long currTime);

#define NUM_BUTTONS 3
Button buttons[NUM_BUTTONS] = {
  Button("Blue (Down)", 10, blueButtonHandler),
  Button("Yellow (Up)", 11, yellowButtonHandler),
  Button("White (Power)", 12, whiteButtonHandler),
};
Motor motor(5, 6);
WIFI wifi("rockerssid", "password", "rocker1");
WebServer webserver(80);
Request currRequest;
bool inWAPMode = false;

void whiteButtonHandler(Button *b, int event, unsigned long currTime) {
  static unsigned long lastTime = 0;
  if (event == BUTTON_UP) {
    // Go into WAP mode if white button was pressed for 5 seconds
    if (currTime - b->downAt() >= 5000) {
      DPRINTLN((String)"Voila Kicking off WAP mode");
      inWAPMode = true;
      wifi.stop();
      wifi.configure("rockerssid", "password", "rocker1");
      wifi.start(0);
      webserver.stop();
    }
  } else if (currTime - lastTime >= 1000) {
    lastTime = currTime;
    DPRINTLN((String)"White Button: " + event + ", Time: " + currTime);
  }
}

void blueButtonHandler(Button *b, int event, unsigned long currTime) {
  if (event == BUTTON_UP) {
    // Go into WAP mode if white button was pressed for 5 seconds
    motor.decreaseSpeed();
  }
}

void yellowButtonHandler(Button *b, int event, unsigned long currTime) {
  if (event == BUTTON_UP) {
    // Go into WAP mode if white button was pressed for 5 seconds
    motor.increaseSpeed();
  }
}

void setup()
{
#ifdef DEBUG 
    Serial.begin(9600);
    // wait for serial port to connect. Needed for native USB port only
    while (!Serial) ;
#endif
  DPRINTLN("Here.....");
  motor.setup();
  for (int i = 0;i < NUM_BUTTONS;i++) {
    buttons[i].setup();
  }
  webserver.onRequest = onRequest;
  webserver.onBodyStarted = onBodyStarted;

  if (wifi.loadFromEEPROM()) {
    wifi.start(2);
  }
  webserver.start();
}

void loop() {
  if (inWAPMode) {
    wifi.handleAPClient();
  }
  webserver.handleClient();
  // now handle buttons
  for (int i = 0;i < NUM_BUTTONS;i++) {
    buttons[i].next();
  }
}

void startResponse(
  WiFiClient &client, int statusCode,
  const char *statusMessage, const char *contentType
) {
    client.print("HTTP/1.1 "); client.print(statusCode); client.println(statusMessage);
    client.print("Content-type: "); client.println(contentType);
    client.println();
}

void* onRequest(String method, String path, String version) {
  currRequest.method = method;
  currRequest.path = path;
  return &currRequest;
}

void onBodyStarted(WiFiClient &client, void *reqctx) {
  Request *req = (Request *)reqctx;
  Serial.println("Request: ");
  Serial.println(req->method + String(": [") + req->path + String("]"));

  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
        // Check to see if the client request was "GET /H" or "GET /L":
  int led = LED_BUILTIN;
  if (req->path == "/H") {
    startResponse(client, 200, "OK", "text/html");
    digitalWrite(led, HIGH);               // GET /H turns the LED on
  } else if (req->path == "/L") {
    startResponse(client, 200, "OK", "text/html");
    digitalWrite(led, LOW);                // GET /L turns the LED off
  } else if (req->path.startsWith("/setupwifi?")) {
    String path = req->path.substring(11);  // strip the "/setupwifi?"
    String ssid = "";
    String password = "";
    String type = "";
    String hostname = "";
    while (true) {
      int eqPos = path.indexOf("=");
      if (eqPos < 0) break;
      String key = path.substring(0, eqPos);
      int ampPos = path.indexOf("&", eqPos);
      String value = ampPos < 0 ?
                        path.substring(eqPos + 1) :
                        path.substring(eqPos + 1, ampPos);
      Serial.println((String)"Key: " + key + ", Value: " + value);
      if (key == "ssid") { ssid = value; }
      if (key == "password") { password = value; }
      if (key == "hostname") { hostname = value; }
      if (ampPos < 0) break;
      path = path.substring(ampPos + 1);
    }
    int mode = 2;
    if (ssid == "") {
      startResponse(client, 400, "InvalidSSID", "text/html");
    } else {
      client.print("Restarting with new credentials...");
      wifi.configure(ssid, password, hostname);
      wifi.saveToEEPROM();
      wifi.stop();
      wifi.start(2);
    }
  } else {
    // the content of the HTTP response follows the header:
    startResponse(client, 200, "OK", "text/html");
    client.print("Click <a href=\"/H\">here</a> turn the LED on<br>");
    client.print("Click <a href=\"/L\">here</a> turn the LED off<br>");

    int randomReading = analogRead(A1);
    client.print("Random reading from analog pin: ");
    client.print(randomReading);
    client.print("<br>Method: "); client.print(req->method); client.print("</br>");
    client.print("<br>Path: "); client.print(req->path); client.print("</br>");
  }
  // The HTTP response ends with another blank line:
  client.println();
}
