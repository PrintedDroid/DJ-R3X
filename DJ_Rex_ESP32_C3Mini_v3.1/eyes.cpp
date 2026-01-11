#include "eyes.h"
#include "helpers.h"

void initializeEyes() {
    for (byte x = 0; x < NUM_EYES; x++) {
        EyesIntervalTime[x] = random(eyeFlickerMinTime, eyeFlickerMaxTime);
        EyesLEDMillis[x] = millis();
        EyesLEDOn[x] = 0;
        EyesLEDBrightness[x] = ledBrightness;
        EyesLEDMinBrightness[x] = ledBrightness;
    }
}

void updateEyes() {
    CRGB eyeColors[NUM_EYES];

    // Determine the color for each eye based on the current eyeMode
    switch (eyeMode) {
        case 0: // Single Color
            eyeColors[0] = getColor(eyeColorIndex);
            eyeColors[1] = getColor(eyeColorIndex);
            break;

        case 1: // Dual Color
            eyeColors[0] = getColor(eyeColorIndex);  // Eye 1 (Right) is the primary color
            eyeColors[1] = getColor(eyeColorIndex2); // Eye 2 (Left) is the secondary color
            break;

        case 2: // Alternating
            {
                static bool alternateState = false;
                // Flip the state every 500ms
                EVERY_N_MILLISECONDS(500) {
                    alternateState = !alternateState;
                }

                if (alternateState) {
                    eyeColors[0] = getColor(eyeColorIndex);
                    eyeColors[1] = getColor(eyeColorIndex2);
                } else {
                    eyeColors[0] = getColor(eyeColorIndex2);
                    eyeColors[1] = getColor(eyeColorIndex);
                }
            }
            break;
        
        default: // Fallback to Single Color
            eyeColors[0] = getColor(eyeColorIndex);
            eyeColors[1] = getColor(eyeColorIndex);
            break;
    }

    // Apply the flicker or static logic using the determined colors
    if (!eyeFlickerEnabled) {
        // Static eyes mode
        for (int pos = 0; pos < NUM_EYES; pos++) {
            DJLEDs_Eyes[pos] = eyeColors[pos]; // Use the color determined by eyeMode

            // Apply eye brightness boost
            DJLEDs_Eyes[pos].r = min(255, (DJLEDs_Eyes[pos].r * eyeBrightness) / 100);
            DJLEDs_Eyes[pos].g = min(255, (DJLEDs_Eyes[pos].g * eyeBrightness) / 100);
            DJLEDs_Eyes[pos].b = min(255, (DJLEDs_Eyes[pos].b * eyeBrightness) / 100);

            // Apply static brightness setting
            DJLEDs_Eyes[pos].fadeToBlackBy(255 - eyeStaticBrightness);
        }
        return;
    }
    
    // Original flicker animation
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
                DJLEDs_Eyes[pos] = eyeColors[pos]; // Use the color determined by eyeMode

                // Apply eye brightness boost
                DJLEDs_Eyes[pos].r = min(255, (DJLEDs_Eyes[pos].r * eyeBrightness) / 100);
                DJLEDs_Eyes[pos].g = min(255, (DJLEDs_Eyes[pos].g * eyeBrightness) / 100);
                DJLEDs_Eyes[pos].b = min(255, (DJLEDs_Eyes[pos].b * eyeBrightness) / 100);
                
                EyesIntervalTime[pos] = random(eyeFlickerMinTime, eyeFlickerMaxTime);
                EyesLEDMillis[pos] = millis();
                EyesLEDOn[pos] = 1;
                EyesLEDMinBrightness[pos] = random(ledBrightness * 0.2, ledBrightness);
            } else {
                EyesIntervalTime[pos] = random(eyeFlickerMinTime, eyeFlickerMaxTime + 400);
                EyesLEDMillis[pos] = millis();
                EyesLEDOn[pos] = 0;
            }
        }
    }
}

// NEW: Function to print current eye flicker settings
void printEyeFlickerSettings() {
    Serial.println(F("\n=== Eye Flicker Settings ==="));
    Serial.print(F("Flicker Enabled: "));
    Serial.println(eyeFlickerEnabled ? "YES" : "NO");
    Serial.print(F("Flicker Min Time: "));
    Serial.print(eyeFlickerMinTime);
    Serial.println(F("ms"));
    Serial.print(F("Flicker Max Time: "));
    Serial.print(eyeFlickerMaxTime);
    Serial.println(F("ms"));
    Serial.print(F("Static Brightness: "));
    Serial.print(eyeStaticBrightness);
    Serial.println(F("/255"));
    Serial.print(F("Eye Color: "));
    Serial.print(eyeColorIndex);
    Serial.print(F(" ("));
    Serial.print(ColorNames[eyeColorIndex]);
    Serial.println(F(")"));
    Serial.print(F("Eye Brightness: "));
    Serial.print(eyeBrightness);
    Serial.println(F("%"));
    Serial.println(F("===========================\n"));
}