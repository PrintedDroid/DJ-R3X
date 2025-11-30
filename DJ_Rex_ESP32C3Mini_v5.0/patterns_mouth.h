#ifndef PATTERNS_MOUTH_H
#define PATTERNS_MOUTH_H

#include "config.h"
#include "globals.h"

// Mouth pattern functions
void updateMouth();
void mouthOff();
void mouthTalk();
void mouthSmile();
void mouthAudioReactive();
void mouthRainbow();
void mouthDebug();

// Helper function for brightness compensation
CRGB adjustMouthBrightness(CRGB color, int row, int ledInRow);

#endif