
#include "Wifi.h"
#include <EEPROM.h>

int led =  LED_BUILTIN;
#define DATA_VERSION 2
#define POST_CONN_WAIT 5000
#define VERSION_ADDRESS 0
#define SSID_ADDRESS 1
#define PASSWD_ADDRESS 33
#define HOSTNAME_ADDRESS 65
#define KEYINDEX_ADDRESS 97

void writeStringToEEPROM(int addrOffset, const String &strToWrite)
{
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

String readStringFromEEPROM(int addrOffset)
{
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0';
  return String(data);
}

WIFI::WIFI(char *s, char *pwd, char *hn, int ki) : ssid(s), password(pwd), hostname(hn), keyIndex(ki) {
  status = WL_IDLE_STATUS;
}

bool WIFI::loadFromEEPROM() {
  if (EEPROM.read(VERSION_ADDRESS) != DATA_VERSION) {
    return false;
  }

  String _ssid = readStringFromEEPROM(SSID_ADDRESS);
  if (_ssid == "") {
    Serial.println("WIFI is not yet configured.");
    return false;
  }
  ssid = _ssid;

  password = readStringFromEEPROM(PASSWD_ADDRESS);
  hostname = readStringFromEEPROM(HOSTNAME_ADDRESS);
  EEPROM.get(KEYINDEX_ADDRESS, keyIndex);
  Serial.println((String)"SSID: " +  ssid);
  Serial.println((String)"Password: " +  password);
  Serial.println((String)"Hostname: " +  hostname);
  Serial.println((String)"KeyIndex: " +  keyIndex);
  return true;
}

void WIFI::saveToEEPROM() {
  Serial.println((String)"Saving SSID: " +  ssid);
  Serial.println((String)"Saving Password: " +  password);
  Serial.println((String)"Saving Hostname: " +  hostname);
  Serial.println((String)"Saving KeyIndex: " +  keyIndex);
  EEPROM.write(VERSION_ADDRESS, DATA_VERSION);
  writeStringToEEPROM(SSID_ADDRESS, ssid);
  writeStringToEEPROM(PASSWD_ADDRESS, password);
  writeStringToEEPROM(HOSTNAME_ADDRESS, hostname);
  EEPROM.write(KEYINDEX_ADDRESS, keyIndex);
}

void WIFI::configure(String s, String passwd, String hname, int ki) {
  ssid = s;
  password = passwd;
  hostname = hname;
  keyIndex = ki;
}

/**
 * Starts wifi in either WAP or client mode.
 */
void WIFI::start(int mode) {
  Serial.println("Starting Access Point...");
  pinMode(led, OUTPUT);      // set the LED pin mode
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  Serial.println((String)"Setting hostname: " + hostname);

  // print the network name (SSID);
  if (mode == 0) {
    IPAddress us(10, 10, 10, 10);
    WiFi.config(us, us, us);
    WiFi.setHostname(hostname.c_str());
    Serial.println((String)"Creating access point: " + ssid);

    // Create open network. Change this line if you want to create an WEP network:
    status = WiFi.beginAP(ssid.c_str(), password.c_str());
    if (status != WL_AP_LISTENING) {
      Serial.println("Creating access point failed");
      // don't continue
      while (true);
    }

    // wait 10 seconds for connection:
    delay(POST_CONN_WAIT);
  } else {
    WiFi.setHostname(hostname.c_str());
    Serial.print("Connecting to access point: ");
    Serial.println(ssid.c_str());

    // attempt to connect to Wifi network:
    for (int numTries = 2;status != WL_CONNECTED && numTries > 0;numTries--) {
      if (mode == 1) { // WEP
        Serial.println((String)"Attempting to connect to WEP network, SSID: " + ssid);
        status = WiFi.begin(ssid.c_str(), keyIndex, password.c_str());
      } else if (mode == 2) { // WPA/WPA2
        Serial.println((String)"Attempting to connect to WPA/WPA2 network, SSID: " + ssid);
        status = WiFi.begin(ssid.c_str(), password.c_str());
      } else if (mode == 3) { // open unencrypted network
        Serial.println((String)"Attempting to connect to open network, SSID: " + ssid);
        status = WiFi.begin(ssid.c_str());
      }
      Serial.print("Conection Status: ");
      Serial.println(status);
      // wait 10 seconds for connection:
      delay(2 * POST_CONN_WAIT);
    }

    if (status == WL_CONNECTED) {
      Serial.println("You're connected to the network");
    } else {
      Serial.print("Connection failed.  Status: ");
      Serial.println(status);
    }
    printCurrentNet();
  }

  // you're connected now, so print out the status
  printWiFiStatus();
}

void WIFI::stop() {
  status = WiFi.disconnect();
  Serial.print("WAP Status: "); Serial.println(status);
}


bool WIFI::handleAPClient() {
    // compare the previous status to the current status
  if (status != WiFi.status()) {
    // it has changed update the variable
    status = WiFi.status();

    if (status == WL_AP_CONNECTED) {
      // a device has connected to the AP
      Serial.println("Device connected to AP");
    } else {
      // a device has disconnected from the AP, and we are back in listening mode
      Serial.println("Device disconnected from AP");
    }
  }
  return status == WL_AP_CONNECTED;
}

WebServer::WebServer(int port) : server(port) {
}

void WebServer::handleClient() {
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

void WIFI::printWAPStatus() {
  // print the SSID of the network you're attached to:
  Serial.println((String)"SSID: " + WiFi.SSID());
  //Serial.println((String)"Hostname: " + WiFi.getHostname());
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

void WIFI::printWiFiStatus() {
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);

  Serial.print("MAC address: ");
  printMacAddress(mac);
}

void WIFI::printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

void WIFI::printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}
