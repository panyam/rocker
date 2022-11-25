
#include "Wifi.h"
#include "eepromutils.h"

#define DATA_VERSION 2
#define POST_CONN_WAIT 5000
#define VERSION_ADDRESS 0
#define SSID_ADDRESS 1
#define PASSWD_ADDRESS 33
#define HOSTNAME_ADDRESS 65
#define KEYINDEX_ADDRESS 97

WIFI::WIFI(char *s, char *pwd, char *hn, int ki) : ssid(s), password(pwd), hostname(hn), keyIndex(ki) {
  status = WL_IDLE_STATUS;
  state = WIFI_IDLE;
}

void WIFI::setup() {
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
bool WIFI::start(int mode, int numRetries) {
  wifiMode = mode;
  numTries = numRetries;
  currTry = 0;
  stop();
  if (mode == 0) {
    state = STARTING_WAP;
  } else {
    state = CONNECTING_TO_WIFI;
  }

  // print the network name (SSID);
  if (mode != 0) {
  }

  // you're connected now, so print out the status
  printWiFiStatus();
  return status == WL_CONNECTED;
}

void WIFI::stop() {
  status = WiFi.disconnect();
  Serial.println(String("WAP Status: ") + status);
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

bool WIFI::tryStartingWAP() {
  Serial.println("Starting Access Point...");
  IPAddress us(10, 10, 10, 10);
  WiFi.config(us, us, us);
  WiFi.setHostname(hostname.c_str());
  Serial.println((String)"Creating access point: " + ssid);

  // Create open network. Change this line if you want to create an WEP network:
  status = WiFi.beginAP(ssid.c_str(), password.c_str());
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    return false;
    // don't continue
    // while (true);
  }
  return true;
}

bool WIFI::tryConnectingToWifi() {
  // attempt to connect to Wifi network:
  Serial.println((String)"Connecting to Wifi via Mode (" + wifiMode + "): SSID: " + ssid + ", Password: " + password + ", Hostname: " + hostname);
  WiFi.setHostname(hostname.c_str());
  if (wifiMode == 1) { // WEP
    status = WiFi.begin(ssid.c_str(), keyIndex, password.c_str());
  } else if (wifiMode == 2) { // WPA/WPA2
    status = WiFi.begin(ssid.c_str(), password.c_str());
  } else if (wifiMode == 3) { // open unencrypted network
    status = WiFi.begin(ssid.c_str());
  }
  Serial.println(String("Conection Status: ") + status + ", Success: " + (status == WL_CONNECTED));
  return status == WL_CONNECTED;
}

void WIFI::next() {
  if (state == WIFI_IDLE) {
    // do nothing
    currTry = 0;
  } else if (state == WAP_STARTED) {
    // what here?
    handleAPClient();
  } else if (state == STARTING_WAP) {
    if (currTry > 0) {
      delay(POST_CONN_WAIT);
    }
    currTry ++;
    bool success = tryStartingWAP();
    if (success || currTry >= numTries) {
      state = success ? WAP_STARTED : WIFI_IDLE;
      if (success) printWiFiStatus();
    }
    if (!success && currTry < numTries) {
      Serial.println(String("WAP failed.  Starting again in: ") + POST_CONN_WAIT);
    }
  } else if (state == CONNECTING_TO_WIFI) {  // we are trying to connect to a wifi router
    if (currTry > 0) {
      delay(POST_CONN_WAIT);
    }
    currTry ++;
    bool success = tryConnectingToWifi();
    if (success || currTry >= numTries) {
      state = success ? WIFI_CONNECTED : WIFI_IDLE;
      printCurrentNet();
    }
    if (!success && currTry < numTries) {
      Serial.println(String("WIFI connection failed.  Trying again in: ") + POST_CONN_WAIT);
    }
  }
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
