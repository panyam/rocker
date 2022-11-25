
#ifndef __EEPROM_UTILS_H__
#define __EEPROM_UTILS_H__

#include "Arduino.h"
#include <EEPROM.h>

extern void writeStringToEEPROM(int addrOffset, const String &strToWrite);
extern String readStringFromEEPROM(int addrOffset);

#endif


