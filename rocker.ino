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
WIFI wifi("rocker_wifi", "password", "rocker1");
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
      wifi.configure("rocker_wifi", "password", "rocker1");
      wifi.start(0);
      webserver.stop();
    } else {
      // turn off the rocker for now
      motor.setSpeed(0);
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

  // if we have a wifi setup then go ahead and use it
  if (wifi.loadFromEEPROM()) {
    wifi.start(2);
  }
  webserver.start();
}

void loop() {
  unsigned long currTime = millis();
  if (inWAPMode) {
    wifi.handleAPClient();

    // blink
    digitalWrite(LED_BUILTIN, currTime % 1000 < 500 ? HIGH : LOW);
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  webserver.handleClient();
  // now handle buttons
  for (int i = 0;i < NUM_BUTTONS;i++) {
    buttons[i].next();
  }
}

void startResponse(
  WiFiClient &client, int statusCode,
  const char *statusMessage, const char *contentType,
  bool noHeaders = true
) {
  client.print("HTTP/1.1 "); client.print(statusCode); client.println(statusMessage);
  client.print("Content-type: "); client.println(contentType);
  if (noHeaders) {
    client.println();
  }
}

void* onRequest(String method, String path, String version) {
  currRequest.method = method;
  currRequest.path = path;
  return &currRequest;
}

String frontPageHtml = " \
  <html> \
    <head> \
    </head> \
    <body> \
      <center><h2>Configure Rocker's WIFI</h2></center> \
      <form action=\"/setupwifi\"> \
        <label for=\"ssid\">SSID:</label> \
        <br/> \
        <input id=\"ssid\" value = \"SITATHEGREAT\" name=\"ssid\" placeholder=\"Enter Your network SSID\"/> \
        <br/> \
        <label for=\"password\">Password:</label> \
        <br/> \
        <input id=\"password\" value = \"GAYATHRIGOWRI\" name=\"password\" placeholder=\"Enter Your network Password\"/> \
        <br/> \
        <label for=\"hostname\">SSID:</label> \
        <br/> \
        <input id=\"hostname\" value=\"rocker1\" name=\"hostname\" placeholder=\"Name your rocker\"/> \
        <br/> \
        <center><button>Submit</button></center> \
      </form> \
    </body> \
  </html> \
";

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
  } else if (req->path.startsWith("/speed/")) {
    int speed = req->path.substring(strlen("/speed/")).toInt();
    motor.setSpeed(speed);
    startResponse(client, 302, "Moved", "", false);
    client.println("Location: /");
    client.println();
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
    if (inWAPMode) {
      client.print(frontPageHtml);
    } else {
      client.println("<html>");
      client.println("  <head>");
      client.println((String)"<title>Control - " + wifi.hostname + "</title>");
      client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
      client.println("<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css\">");
      client.println("  </head>");
      client.println("  <body>");
      client.println("  <center><h2>Control your rocker - " + wifi.hostname + "</h2></center>");
      client.println("  <center>");
      client.println(String("Speed: ") + motor.currSpeed);
      client.println(String("    <form action=\"/speed/") + (motor.currSpeed - 1) + String("\">") + 
                          "<button style=\"font-size:24px\">"
                          "<i class=\"fa fa-minus\" style=\"font-size:48px;color:red\"></i></button></form>");
      client.println(String("    <form action=\"/speed/") + (motor.currSpeed + 1) + String("\">") + 
                          "<button style=\"font-size:24px\">"
                          "<i class=\"fa fa-plus\" style=\"font-size:48px;color:red\"></i></button></form>");
      client.println("    <form action=\"/speed/0\"> "
                          "<button style=\"font-size:24px\">"
                          "<i class=\"fa fa-power-off\" style=\"font-size:48px;color:red\"></i></button></form>");
      client.println("  </center>");
      client.println("  </body>");
      client.println("</html>");
    }
  }
  // The HTTP response ends with another blank line:
  client.println();
}
