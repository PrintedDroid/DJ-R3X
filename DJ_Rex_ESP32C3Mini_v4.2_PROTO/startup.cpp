#include "startup.h"
#include "helpers.h"
#include "patterns_body.h"
#include "patterns_mouth.h"
#include "eyes.h"

void initializeStartup() {
    startupComplete = false;
    startupBeginTime = millis();
    startupPhase = 0;
    
    for (int i = 0; i < 3; i++) {
        panelActive[i] = false;
        panelActivationTime[i] = 0;
    }
    
    // Clear all LEDs
    FastLED.clear();
    FastLED.show();
}

void runStartupSequence() {
    if (startupComplete) return;
    
    unsigned long elapsedTime = millis() - startupBeginTime;
    
    // Phase 1: Flicker and activate panels (0-2000ms)
    if (elapsedTime < 2000) {
        // Panel activation sequence
        if (!panelActive[0] && elapsedTime > 200) {
            panelActive[0] = true;
            panelActivationTime[0] = millis();
            Serial.println(F("Startup: Right panel activating..."));
        }
        if (!panelActive[1] && elapsedTime > 600) {
            panelActive[1] = true;
            panelActivationTime[1] = millis();
            Serial.println(F("Startup: Middle panel activating..."));
        }
        if (!panelActive[2] && elapsedTime > 1000) {
            panelActive[2] = true;
            panelActivationTime[2] = millis();
            Serial.println(F("Startup: Left panel activating..."));
        }
        
        // Handle each panel
        for (int panel = 0; panel < 3; panel++) {
            if (panelActive[panel]) {
                CRGB* leds = getLEDArray(panel);
                unsigned long panelTime = millis() - panelActivationTime[panel];
                
                if (panelTime < 500) {
                    // Flicker phase
                    if (random8() < STARTUP_FLICKER_CHANCE) {
                        // Random LEDs flicker with white/blue
                        for (int i = 0; i < NUM_LEDS_PER_PANEL; i++) {
                            if (random8() < 50) {
                                leds[i] = (random8() < 128) ? 
                                    applyBodyBrightness(CRGB(200, 220, 255)) : // Blue-white
                                    applyBodyBrightness(CRGB::Black);
                            }
                        }
                    }
                } else {
                    // Stabilize to pattern colors
                    fadeToBlackBy(leds, NUM_LEDS_PER_PANEL, 10);
                    
                    // Start showing pattern 16 colors
                    if (random8() < 100) {
                        // Side LEDs - Red/White/Blue
                        uint8_t colorChoice = random8(3);
                        uint8_t colorIndex = (colorChoice == 0) ? 0 : (colorChoice == 1) ? 2 : 3;
                        leds[random8(SIDE_LEDS_COUNT)] = applyBodyBrightness(getColor(colorIndex));
                    }
                    
                    // Blocks start appearing
                    if (panelTime > 800) {
                        static const uint8_t sequenceColors[9] = {
                            3, 2, 2, 3, 2, 2, 2, 2, 3
                        };
                        
                        uint8_t blockStarts[3] = {BLOCK1_START, BLOCK2_START, BLOCK3_START};
                        for (uint8_t blockNum = 0; blockNum < 3; blockNum++) {
                            if (random8() < 30) {
                                uint8_t globalBlockIndex = getGlobalBlockIndex(panel, blockNum);
                                CRGB blockColor = applyBodyBrightness(getColor(sequenceColors[globalBlockIndex]));
                                setBlock(panel, blockStarts[blockNum], blockColor);
                            }
                        }
                    }
                }
            }
        }
        
        // Eyes start flickering
        if (elapsedTime > 500) {
            for (int i = 0; i < NUM_EYES; i++) {
                if (random8() < 100) {
                    DJLEDs_Eyes[i] = getEyeColor(i);
                    DJLEDs_Eyes[i].fadeToBlackBy(random8(100, 200));
                }
            }
        }
        
        // Mouth starts showing
        if (elapsedTime > 1200 && mouthEnabled) {
            fadeToBlackBy(DJLEDs_Mouth, NUM_MOUTH_LEDS, 20);
            if (random8() < 80) {
                // Random mouth LEDs light up
                for (int i = 0; i < 10; i++) {
                    int row = random8(MOUTH_ROWS);
                    int led = random8(mouthRowLeds[row]);
                    CRGB color = getColor(2);  // Use Blue for startup
                    color.fadeToBlackBy(255 - mouthBrightness);
                    DJLEDs_Mouth[mouthRowStart[row] + led] = adjustMouthBrightness(color, row, led);
                }
            }
        }
    }
    // Phase 2: Transition to normal operation (2000-3000ms)
    else if (elapsedTime < STARTUP_DURATION_MS) {
        // Smooth transition to pattern 16
        float transitionProgress = (float)(elapsedTime - 2000) / 1000.0f;
        
        // Reduce flicker, increase pattern visibility
        if (random8() < (255 - (transitionProgress * 200))) {
            // Still some flicker but decreasing
            for (int panel = 0; panel < 3; panel++) {
                CRGB* leds = getLEDArray(panel);
                if (random8() < 20) {
                    leds[random8(NUM_LEDS_PER_PANEL)].fadeToBlackBy(random8(100));
                }
            }
        }
        
        // Pattern becomes more stable
        CustomBlockSequence();
        
        // Eyes stabilize
        updateEyes();
        
        // Mouth stabilizes
        if (mouthEnabled) {
            updateMouth();
        }
    }
    // Phase 3: Complete
    else {
        startupComplete = true;
        Serial.println(F("Startup sequence complete!"));
        Serial.print(F("Starting pattern: "));
        Serial.println(patternNames[currentPattern]);
    }
}

bool isStartupComplete() {
    return startupComplete;
}