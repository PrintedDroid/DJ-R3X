#ifndef HELPERS_H
#define HELPERS_H

#include "config.h"
#include "globals.h"

// Initialization
void initializeHelpers();

// LED array access
CRGB* getLEDArray(uint8_t panel);
uint8_t getTimingIndex(uint8_t panel, uint8_t pos);

// Block operations
void setBlock(uint8_t panel, uint8_t blockStart, CRGB color);
void fadeBlock(uint8_t panel, uint8_t blockStart, uint8_t fadeAmount);
uint8_t getGlobalBlockIndex(uint8_t panel, uint8_t localBlock);

// Color operations
CRGB getColor(uint8_t colorIndex);
CRGB getSideLEDColor();
CRGB getBlockColor(uint8_t blockIndex);
CRGB applyBodyBrightness(CRGB color);

// Timing functions
uint16_t getRandomTiming(uint16_t minTime, uint16_t maxTime);
uint16_t getRandomTimingWithRate(uint16_t minTime, uint16_t maxTime, uint8_t rate);

// Smooth color transition helper
CRGB smoothColorTransition(CRGB currentColor, CRGB targetColor, uint8_t steps);

void printEyeFlickerSettings(); 
void printLEDPerformanceStats();
void resetLEDPerformanceStats();
bool isLEDOperationSafe();
void emergencyLEDClear();
void validateHelperState();

#endif