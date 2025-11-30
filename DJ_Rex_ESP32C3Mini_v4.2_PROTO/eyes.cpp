#include "eyes.h"
#include "helpers.h"

void initializeEyes() {
    for (byte x = 0; x < NUM_EYES; x++) {
        EyesIntervalTime[x] = random(200, 1600);
        EyesLEDMillis[x] = millis();
        EyesLEDOn[x] = 0;
        EyesLEDBrightness[x] = ledBrightness;
        EyesLEDMinBrightness[x] = ledBrightness;
    }
}

CRGB getEyeColor(uint8_t eyeIndex) {
    CRGB color;
    
    switch (eyeMode) {
        case EYE_MODE_SINGLE:
            // Both eyes use primary color
            color = getColor(eyeColorIndex);
            break;
            
        case EYE_MODE_DUAL:
            // Each eye uses its own color
            if (eyeIndex == 0) {
                color = getColor(eyeColorIndex);
            } else {
                color = getColor(eyeColorIndex2);
            }
            break;
            
        case EYE_MODE_ALTERNATE:
            // Eyes alternate between two colors
            static unsigned long lastAlternate = 0;
            static bool alternateState = false;
            
            if (millis() - lastAlternate > 2000) { // Switch every 2 seconds
                lastAlternate = millis();
                alternateState = !alternateState;
            }
            
            if ((eyeIndex == 0 && !alternateState) || (eyeIndex == 1 && alternateState)) {
                color = getColor(eyeColorIndex);
            } else {
                color = getColor(eyeColorIndex2);
            }
            break;
            
        default:
            color = getColor(eyeColorIndex);
            break;
    }
    
    // Apply eye brightness boost
    color.r = min(255, (color.r * eyeBrightness) / 100);
    color.g = min(255, (color.g * eyeBrightness) / 100);
    color.b = min(255, (color.b * eyeBrightness) / 100);
    
    return color;
}

void updateEyes() {
    if (!eyeFlickerEnabled) {
        // Static mode - Eyes always on with configured brightness
        for (int pos = 0; pos < NUM_EYES; pos++) {
            CRGB eyeColor = getEyeColor(pos);
            
            // Apply static brightness instead of dynamic brightness
            eyeColor.r = (eyeColor.r * eyeStaticBrightness) / 255;
            eyeColor.g = (eyeColor.g * eyeStaticBrightness) / 255;
            eyeColor.b = (eyeColor.b * eyeStaticBrightness) / 255;
            
            DJLEDs_Eyes[pos] = eyeColor;
        }
        return;
    }
    
    // Original flicker logic with configurable timing
    for (int pos = 0; pos < NUM_EYES; pos++) {
        if (!EyesLEDOn[pos]) {
            DJLEDs_Eyes[pos].maximizeBrightness(EyesLEDBrightness[pos]);
            if (EyesLEDBrightness[pos] < ledBrightness) EyesLEDBrightness[pos]++;
        } else {
            DJLEDs_Eyes[pos].maximizeBrightness(EyesLEDBrightness[pos]);
            if (EyesLEDBrightness[pos] > EyesLEDMinBrightness[pos]) EyesLEDBrightness[pos]--;
        }
        
        if (millis() - EyesLEDMillis[pos] > EyesIntervalTime[pos]) {
            if (!EyesLEDOn[pos]) {
                DJLEDs_Eyes[pos] = getEyeColor(pos);
                
                // Use configurable flicker timing instead of hardcoded values
                EyesIntervalTime[pos] = random(eyeFlickerMinTime, eyeFlickerMaxTime);
                EyesLEDMillis[pos] = millis();
                EyesLEDOn[pos] = 1;
                EyesLEDMinBrightness[pos] = random(ledBrightness * 0.2, ledBrightness);
            } else {
                // Use configurable timing for off period too
                EyesIntervalTime[pos] = random(eyeFlickerMinTime, eyeFlickerMaxTime + 400);
                EyesLEDMillis[pos] = millis();
                EyesLEDOn[pos] = 0;
            }
        }
    }
} // <<--- DIESE KLAMMER HAT GEFEHLT UND BEENDET DIE updateEyes() FUNKTION

// Diese Funktion steht jetzt für sich allein, außerhalb von updateEyes()
void printEyeFlickerSettings() {
    Serial.println(F("--- Eye Flicker Settings ---"));
    Serial.print(F("  Flicker Enabled: "));
    Serial.println(eyeFlickerEnabled ? "ON" : "OFF");

    if (eyeFlickerEnabled) {
        Serial.print(F("  Min Time: "));
        Serial.print(eyeFlickerMinTime);
        Serial.println(F(" ms"));
        Serial.print(F("  Max Time: "));
        Serial.print(eyeFlickerMaxTime);
        Serial.println(F(" ms"));
    } else {
        Serial.print(F("  Static Brightness: "));
        Serial.println(eyeStaticBrightness);
    }
    Serial.println(F("--------------------------"));
}