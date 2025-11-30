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
void mouthWave();
void mouthPulse();
void mouthSparkle();
void mouthScanline();
void mouthFirework();
void mouthAudioVU();
void mouthMatrix();
void mouthHeartbeat();
void mouthSpectrum();

// Helper functions
CRGB adjustMouthBrightness(CRGB color, int row, int ledInRow);
bool isLeftSide(int row, int ledInRow);
CRGB getMouthColor(int row, int ledInRow);

#endif