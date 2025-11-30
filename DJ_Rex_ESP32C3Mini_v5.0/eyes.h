#ifndef EYES_H
#define EYES_H

#include "config.h"
#include "globals.h"

void initializeEyes();
void updateEyes();
CRGB getEyeColor(uint8_t eyeIndex);
void printEyeFlickerSettings();

#endif