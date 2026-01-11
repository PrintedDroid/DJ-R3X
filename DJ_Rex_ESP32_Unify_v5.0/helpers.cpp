#include "helpers.h"

void initializeHelpers() {
    // Initialize random seed
    randomSeed(esp_random());
    
    // Initialize timing arrays
    for (byte x = 0; x < TOTAL_BODY_LEDS; x++) {
        IntervalTime[x] = random16(3000);
        LEDMillis[x] = millis();
        LEDOn[x] = 0;
    }
    
    // Initialize matrix drops
    for (int panel = 0; panel < 3; panel++) {
        for (int led = 0; led < 8; led++) {
            matrixDrops[panel][led] = 255;
            matrixBright[panel][led] = 0;
        }
    }
}

CRGB* getLEDArray(uint8_t panel) {
    switch(panel) {
        case 0: return DJLEDs_Right;
        case 1: return DJLEDs_Middle;
        case 2: return DJLEDs_Left;
        default: return DJLEDs_Right;
    }
}

uint8_t getTimingIndex(uint8_t panel, uint8_t pos) {
    return panel * NUM_LEDS_PER_PANEL + pos;
}

void setBlock(uint8_t panel, uint8_t blockStart, CRGB color) {
    CRGB* leds = getLEDArray(panel);
    for (byte i = 0; i < LEDS_PER_BLOCK; i++) {
        leds[blockStart + i] = color;
    }
}

void fadeBlock(uint8_t panel, uint8_t blockStart, uint8_t fadeAmount) {
    CRGB* leds = getLEDArray(panel);
    for (byte i = 0; i < LEDS_PER_BLOCK; i++) {
        leds[blockStart + i].fadeToBlackBy(fadeAmount);
    }
}

CRGB getColor(uint8_t colorIndex) {
    // Handle random color generation for any index >= NUM_STANDARD_COLORS-1
    if (colorIndex >= NUM_STANDARD_COLORS) {
        return CHSV(random8(), 255, 255);  // Nur für Index 20 und höher
    } else {
        return StandardColors[colorIndex];  // Für Index 0-19 statische Farben
    }
}

CRGB getSideLEDColor() {
    switch (sideColorMode) {
        case 0: // Random from 3
            {
                uint8_t colors[3] = {sideColor1, sideColor2, sideColor3};
                return getColor(colors[random(3)]);
            }
        case 1: // Cycle through 3
            {
                uint8_t colors[3] = {sideColor1, sideColor2, sideColor3};
                CRGB color = getColor(colors[sideColorCycleIndex]);
                sideColorCycleIndex = (sideColorCycleIndex + 1) % 3;
                return color;
            }
        case 2: return getColor(sideColor1);
        case 3: return getColor(sideColor2);
        case 4: return getColor(sideColor3);
        default: return getColor(sideColor1);
    }
}

CRGB getBlockColor(uint8_t blockIndex) {
    if (blockIndex >= 9) blockIndex = 0;
    return getColor(blockColors[blockIndex]);
}

uint8_t getGlobalBlockIndex(uint8_t panel, uint8_t localBlock) {
    uint8_t reversedPanel = 2 - panel;
    return reversedPanel * 3 + localBlock;
}

uint16_t getRandomTiming(uint16_t minTime, uint16_t maxTime) {
    uint16_t baseTime = random(minTime, maxTime);
    uint32_t adjustedTime = (baseTime * 256UL) / effectSpeed;
    return constrain(adjustedTime, 50, 30000);
}

uint16_t getRandomTimingWithRate(uint16_t minTime, uint16_t maxTime, uint8_t rate) {
    uint16_t baseTime = random(minTime, maxTime);
    uint32_t adjustedTime = (baseTime * 256UL) / rate;
    return constrain(adjustedTime, 50, 30000);
}

CRGB applyBodyBrightness(CRGB color) {
    CRGB adjustedColor = color;
    adjustedColor.r = min(255, (adjustedColor.r * bodyBrightness) / 100);
    adjustedColor.g = min(255, (adjustedColor.g * bodyBrightness) / 100);
    adjustedColor.b = min(255, (adjustedColor.b * bodyBrightness) / 100);
    return adjustedColor;
}