
// status_led.h
#ifndef STATUS_LED_H
#define STATUS_LED_H

#include "config.h"

void initStatusLED();
void updateStatusLED();
void setStatusLED(StatusLEDPattern pattern);
StatusLEDPattern getStatusLED();

#endif