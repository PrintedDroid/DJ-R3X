#include "demo.h"
#include "helpers.h"

// v5.0: Extended demo patterns list (skip Off pattern and some less interesting ones)
static const uint8_t demoBodyPatterns[] = {
    1,  // Random Blocks
    5,  // Rainbow
    6,  // Rainbow Glitter
    7,  // Confetti
    8,  // Juggle
    11, // Knight Rider
    12, // Breathing
    13, // Matrix Rain
    17, // Plasma (v5.0)
    18, // Fire (v5.0)
    19, // Twinkle (v5.0)
    9,  // Audio Sync
    15  // Audio VU Meter
};
static const uint8_t numDemoBodyPatterns = sizeof(demoBodyPatterns) / sizeof(demoBodyPatterns[0]);

// v5.0: Extended mouth demo patterns
static const uint8_t demoMouthPatterns[] = {
    1,  // Talk
    2,  // Smile
    4,  // Rainbow
    6,  // Wave
    7,  // Pulse
    11, // Sparkle
    12, // Matrix (v5.0)
    13, // Heartbeat (v5.0)
    3,  // Audio Reactive
    14  // Spectrum (v5.0)
};
static const uint8_t numDemoMouthPatterns = sizeof(demoMouthPatterns) / sizeof(demoMouthPatterns[0]);

void handleDemoMode() {
    if (!demoMode) return;

    if (millis() - lastDemoChange >= (demoTime * 1000UL)) {
        lastDemoChange = millis();
        demoStep++;

        // Cycle through demo body patterns
        static uint8_t bodyDemoIndex = 0;
        bodyDemoIndex = (bodyDemoIndex + 1) % numDemoBodyPatterns;
        currentPattern = demoBodyPatterns[bodyDemoIndex];

        Serial.print(F("Demo: "));
        Serial.println(patternNames[currentPattern]);

        // Cycle mouth patterns with body patterns
        static uint8_t mouthDemoIndex = 0;
        mouthDemoIndex = (mouthDemoIndex + 1) % numDemoMouthPatterns;
        mouthPattern = demoMouthPatterns[mouthDemoIndex];

        Serial.print(F("  Mouth: "));
        Serial.println(MouthPatternNames[mouthPattern]);

        // Vary colors every few cycles
        if (demoStep % 3 == 0) {
            solidColorIndex = random8(NUM_STANDARD_COLORS - 1);
            mouthColorIndex = random8(NUM_STANDARD_COLORS - 1);
        }
    }
}