
#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "Arduino.h"

class Button;

enum ButtonEvent {
  /**
   * Called when was button went down.
   * This gives the user a chance to "start" some thing.
   */
  BUTTON_DOWN,

  /**
   * Called when the button is held down (taking debounce filters into account).
   * This gives the user a chance to do something when button is held for 
   * a certain period.
   */
  BUTTON_HELD,

  /**
   * Called when the button was released to mark the end of a button event.
   */
  BUTTON_UP,
};

typedef void (*ButtonHandler)(Button *b, int event, unsigned long currTime);

class Button {
public:
  Button(String name, int pin, unsigned int dbDelay = 50);
  // Reads the value of the pin.
  void setup();
  void reset();
  void next();
  int pin() { return _pin; }
  unsigned long downAt() { return _downAt; }
  String debugInfo();

public:
  String name;
  unsigned int debounceDelay;
  ButtonHandler eventHandler;

private:
  int _pin;
  unsigned long _downAt;
  int lastState;
  int currState;
  unsigned long lastDebouncedAt;
};

#endif
