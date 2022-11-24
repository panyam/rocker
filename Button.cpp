#include "Button.h"
#define DEBUG
#include "common.h"

void defaultButtonHandler(Button *b, int event, unsigned long currTime) {
  static unsigned long lastTime = 0;
  static Button *lastButton = NULL;
  if (currTime > lastTime + 1000 || event == BUTTON_UP || b != lastButton) {
    lastTime = currTime;
    lastButton = b;
    DPRINTLN((String)"Button: (" + b->name + "), " + event + ", Time: " + currTime + ", DownAt: " + b->downAt());
  }
}

Button::Button(String n, int p, ButtonHandler handler): Button(n, p, 50, handler) {
}

Button::Button(String n, int p, unsigned int dbDelay, ButtonHandler handler): name(n), _pin(p), lastState(LOW), currState(LOW), lastDebouncedAt(0), debounceDelay(dbDelay), _downAt(0), eventHandler(handler) {
  if (eventHandler == NULL) {
    eventHandler = defaultButtonHandler;
  }
}

void Button::setup() {
  pinMode(_pin, INPUT);
}

void Button::reset() {
  _downAt = 0;
  lastState = currState = LOW;
  lastDebouncedAt = 0;
}

String Button::debugInfo() {
  return (String)"CurrState: " + currState + ", LastState: " + lastState + ", LastTime: " + lastDebouncedAt;
}

void Button::next() {
  int reading = digitalRead(_pin);
  // see if there was actually a state changed
  unsigned long currTime = millis();
  if ((currTime - lastDebouncedAt) > debounceDelay) {
    if (reading != lastState) {
    // DPRINTLN((String)"1. Reading: " + reading + ", CurrTime: " + currTime + ", " + debugInfo());
      lastDebouncedAt = currTime;
      currState = reading;
      DPRINTLN((String)"Reading: " + reading + ", CurrTime: " + currTime + ", " + debugInfo() + ", " + _downAt);
      if (reading == HIGH) {
        _downAt = currTime;
        eventHandler(this, BUTTON_DOWN, currTime);
      } else if (_downAt > 0) {
        eventHandler(this, BUTTON_UP, currTime);
        _downAt = 0;
      }
    } else if (reading == HIGH && _downAt > 0) {
      eventHandler(this, BUTTON_HELD, currTime);
    } else if (_downAt > 0) {
      // check if this was "just" released?
      // DPRINTLN((String)"Reading: " + reading + ", DownAt: " + _downAt);
    } else {
      // we are non-pressed and non-released so do nothing
    }
  } /**/
  lastState = reading;
}
