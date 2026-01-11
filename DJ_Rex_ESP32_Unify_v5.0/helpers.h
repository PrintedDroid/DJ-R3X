#ifndef HELPERS_H
#define HELPERS_H

#include "config.h"
#include "globals.h"

// Initialize helpers
void initializeHelpers();

// LED array helpers
CRGB* getLEDArray(uint8_t panel);
uint8_t getTimingIndex(uint8_t panel, uint8_t pos);
void setBlock(uint8_t panel, uint8_t blockStart, CRGB color);
void fadeBlock(uint8_t panel, uint8_t blockStart, uint8_t fadeAmount);

// Color helpers
CRGB getColor(uint8_t colorIndex);
CRGB getSideLEDColor();
CRGB getBlockColor(uint8_t blockIndex);
uint8_t getGlobalBlockIndex(uint8_t panel, uint8_t localBlock);
CRGB applyBodyBrightness(CRGB color);

// Timing helpers
uint16_t getRandomTiming(uint16_t minTime, uint16_t maxTime);
uint16_t getRandomTimingWithRate(uint16_t minTime, uint16_t maxTime, uint8_t rate);

#endif