
#include "Rocker.h"

Rocker::Rocker() : motor(0, 0) {
}

/**
 * Performs the next iteration of the run loop.
 */
void Rocker::next() {
  if (inWAPMode) {
    wifi.handleAPClient();
  }
  webserver.handleClient();
}
