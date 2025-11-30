#include "demo.h"
#include "helpers.h"

// External variables for state preservation
extern uint8_t savedPattern;
extern bool savedDemoState;

void handleDemoMode() {
    // Save current pattern when entering demo mode
    if (!savedDemoState && demoMode) {
        savedPattern = currentPattern;
        savedDemoState = true;
        Serial.print(F("> Demo mode started, saved pattern: "));
        Serial.println(patternNames[savedPattern]);
    }
    
    // Restore pattern when exiting demo mode
    if (savedDemoState && !demoMode) {
        currentPattern = savedPattern;
        savedDemoState = false;
        Serial.print(F("> Demo mode ended, restored pattern: "));
        Serial.println(patternNames[currentPattern]);
        return;
    }
    
    if (!demoMode) return;
    
    // Validate demo pattern index
    if (demoPatternIndex >= NUM_PATTERNS) {
        demoPatternIndex = 1;  // Skip pattern 0 (LEDs Off)
    }
    
    if (millis() - lastDemoChange >= (demoTime * 1000UL)) {
        lastDemoChange = millis();
        demoStep++;
        
        // Check if current pattern has color variations
        if (patternProps[demoPatternIndex].hasColorVariations && demoColorIndex < 9) {
            demoColorIndex++;
            
            // Update the color index
            if (patternProps[demoPatternIndex].colorIndexPtr != nullptr) {
                *patternProps[demoPatternIndex].colorIndexPtr = demoColorIndex;
                Serial.print(F("> Demo: "));
                Serial.print(patternProps[demoPatternIndex].colorName);
                Serial.print(F(" - "));
                Serial.println(ColorNames[demoColorIndex]);
            }
            
            // Special handling for confetti pattern (has two colors)
            if (demoPatternIndex == 4) {
                confettiColor2 = (demoColorIndex + 1) % 10;
                Serial.print(F("  Confetti Color 2: "));
                Serial.println(ColorNames[confettiColor2]);
            }
        } else {
            // Move to next pattern
            demoColorIndex = 0;
            demoPatternIndex++;
            
            // Wrap around to pattern 1 (skip LEDs Off)
            if (demoPatternIndex >= NUM_PATTERNS) {
                demoPatternIndex = 1;  // Skip pattern 0 (LEDs Off)
            }
            
            // Validate pattern index
            if (demoPatternIndex >= NUM_PATTERNS) {
                Serial.println(F("! Error: Demo pattern index out of bounds"));
                demoPatternIndex = 1;
            }
            
            currentPattern = demoPatternIndex;
            Serial.print(F("> Demo: Pattern "));
            Serial.print(demoPatternIndex);
            Serial.print(F(" - "));
            Serial.println(patternNames[demoPatternIndex]);
        }
        
        // Cycle mouth patterns too
        static uint8_t mouthDemoIndex = 0;
        mouthDemoIndex = (mouthDemoIndex + 1) % NUM_MOUTH_PATTERNS;
        
        // Skip mouth off pattern
        if (mouthDemoIndex == 0) {
            mouthDemoIndex = 1;
        }
        
        mouthPattern = mouthDemoIndex;
        Serial.print(F("  Mouth: "));
        Serial.print(mouthDemoIndex);
        Serial.print(F(" - "));
        Serial.println(MouthPatternNames[mouthPattern]);
        
        // Randomly change eye mode every few patterns
        if (demoStep % 3 == 0) {
            eyeMode = (eyeMode + 1) % 3;
            Serial.print(F("  Eye Mode: "));
            Serial.println(EyeModeNames[eyeMode]);
        }
    }
}