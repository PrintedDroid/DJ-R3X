#include "demo.h"
#include "helpers.h"

void handleDemoMode() {
    if (!demoMode) return;
    
    if (millis() - lastDemoChange >= (demoTime * 1000UL)) {
        lastDemoChange = millis();
        demoStep++;
        
        // Patterns with color variations
        if ((demoPatternIndex == 2 || demoPatternIndex == 3 || demoPatternIndex == 4 || 
             demoPatternIndex == 10 || demoPatternIndex == 11 || demoPatternIndex == 12 ||
             demoPatternIndex == 13 || demoPatternIndex == 14) && demoColorIndex < 9) {
            demoColorIndex++;
            
            if (demoPatternIndex == 2) {
                solidColorIndex = demoColorIndex;
                Serial.print(F("Demo: Solid Color - "));
                Serial.println(ColorNames[demoColorIndex]);
            } else if (demoPatternIndex == 3) {
                shortColorIndex = demoColorIndex;
                Serial.print(F("Demo: Short Circuit - "));
                Serial.println(ColorNames[demoColorIndex]);
            } else if (demoPatternIndex == 4) {
                confettiColor1 = demoColorIndex;
                confettiColor2 = (demoColorIndex + 1) % 10;
                Serial.print(F("Demo: Confetti - "));
                Serial.print(ColorNames[demoColorIndex]);
                Serial.print(F(" + "));
                Serial.println(ColorNames[confettiColor2]);
            } else if (demoPatternIndex == 10) {
                flashColorIndex = demoColorIndex;
                Serial.print(F("Demo: Solid Flash - "));
                Serial.println(ColorNames[demoColorIndex]);
            } else if (demoPatternIndex == 11) {
                knightColorIndex = demoColorIndex;
                Serial.print(F("Demo: Knight Rider - "));
                Serial.println(ColorNames[demoColorIndex]);
            } else if (demoPatternIndex == 12) {
                breathingColorIndex = demoColorIndex;
                Serial.print(F("Demo: Breathing - "));
                Serial.println(ColorNames[demoColorIndex]);
            } else if (demoPatternIndex == 13) {
                matrixColorIndex = demoColorIndex;
                Serial.print(F("Demo: Matrix Rain - "));
                Serial.println(ColorNames[demoColorIndex]);
            } else if (demoPatternIndex == 14) {
                strobeColorIndex = demoColorIndex;
                Serial.print(F("Demo: Strobe - "));
                Serial.println(ColorNames[demoColorIndex]);
            }
        } else {
            // Move to next pattern
            demoColorIndex = 0;
            demoPatternIndex++;
            
            if (demoPatternIndex >= NUM_PATTERNS) {
                demoPatternIndex = 1;
            }
            
            currentPattern = demoPatternIndex;
            Serial.print(F("Demo: "));
            Serial.println(patternNames[demoPatternIndex]);
        }
        
        // Cycle mouth patterns too
        static uint8_t mouthDemoIndex = 0;
        mouthDemoIndex = (mouthDemoIndex + 1) % 5;
        mouthPattern = mouthDemoIndex;
        Serial.print(F("  Mouth: "));
        Serial.println(MouthPatternNames[mouthPattern]);
    }
}