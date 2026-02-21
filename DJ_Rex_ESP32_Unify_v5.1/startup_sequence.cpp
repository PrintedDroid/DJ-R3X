// startup_sequence.cpp - v5.0 Animated Boot Sequence
#include "startup_sequence.h"
#include "helpers.h"
#include <Arduino.h>

StartupSequence startupSequence;

void StartupSequence::begin() {
    currentPhase = PHASE_INIT;
    phaseStartTime = millis();
    sweepPosition = 0;
    complete = false;
    Serial.println(F("Startup sequence begin"));
}

void StartupSequence::run() {
    if (complete) return;
    runPhase();
}

bool StartupSequence::isComplete() {
    return complete;
}

void StartupSequence::runPhase() {
    unsigned long elapsed = millis() - phaseStartTime;

    switch (currentPhase) {
        case PHASE_INIT:
            // Clear all LEDs
            fill_solid(DJLEDs_Right, NUM_LEDS_PER_PANEL, CRGB::Black);
            fill_solid(DJLEDs_Middle, NUM_LEDS_PER_PANEL, CRGB::Black);
            fill_solid(DJLEDs_Left, NUM_LEDS_PER_PANEL, CRGB::Black);
            fill_solid(DJLEDs_Eyes, NUM_EYES, CRGB::Black);
            fill_solid(DJLEDs_Mouth, NUM_MOUTH_LEDS, CRGB::Black);
            FastLED.show();
            if (elapsed > 200) nextPhase();
            break;

        case PHASE_LED_TEST:
            // Quick red flash on all LEDs
            if (elapsed < 150) {
                fill_solid(DJLEDs_Right, NUM_LEDS_PER_PANEL, CRGB::Red);
                fill_solid(DJLEDs_Middle, NUM_LEDS_PER_PANEL, CRGB::Red);
                fill_solid(DJLEDs_Left, NUM_LEDS_PER_PANEL, CRGB::Red);
                fill_solid(DJLEDs_Eyes, NUM_EYES, CRGB::Red);
                fill_solid(DJLEDs_Mouth, NUM_MOUTH_LEDS, CRGB::Red);
            } else if (elapsed < 300) {
                fill_solid(DJLEDs_Right, NUM_LEDS_PER_PANEL, CRGB::Green);
                fill_solid(DJLEDs_Middle, NUM_LEDS_PER_PANEL, CRGB::Green);
                fill_solid(DJLEDs_Left, NUM_LEDS_PER_PANEL, CRGB::Green);
                fill_solid(DJLEDs_Eyes, NUM_EYES, CRGB::Green);
                fill_solid(DJLEDs_Mouth, NUM_MOUTH_LEDS, CRGB::Green);
            } else if (elapsed < 450) {
                fill_solid(DJLEDs_Right, NUM_LEDS_PER_PANEL, CRGB::Blue);
                fill_solid(DJLEDs_Middle, NUM_LEDS_PER_PANEL, CRGB::Blue);
                fill_solid(DJLEDs_Left, NUM_LEDS_PER_PANEL, CRGB::Blue);
                fill_solid(DJLEDs_Eyes, NUM_EYES, CRGB::Blue);
                fill_solid(DJLEDs_Mouth, NUM_MOUTH_LEDS, CRGB::Blue);
            } else {
                fill_solid(DJLEDs_Right, NUM_LEDS_PER_PANEL, CRGB::Black);
                fill_solid(DJLEDs_Middle, NUM_LEDS_PER_PANEL, CRGB::Black);
                fill_solid(DJLEDs_Left, NUM_LEDS_PER_PANEL, CRGB::Black);
                fill_solid(DJLEDs_Eyes, NUM_EYES, CRGB::Black);
                fill_solid(DJLEDs_Mouth, NUM_MOUTH_LEDS, CRGB::Black);
                nextPhase();
            }
            FastLED.show();
            break;

        case PHASE_EYES_ON:
            // Eyes fade in orange
            {
                uint8_t brightness = min(255, (int)(elapsed * 255 / 400));
                CRGB eyeColor = CRGB::OrangeRed;
                eyeColor.fadeToBlackBy(255 - brightness);
                fill_solid(DJLEDs_Eyes, NUM_EYES, eyeColor);
                FastLED.show();
                if (elapsed > 400) nextPhase();
            }
            break;

        case PHASE_MOUTH_ON:
            // Mouth rows light up from center
            {
                int rowsToLight = min(MOUTH_ROWS / 2, (int)(elapsed / 50));
                fill_solid(DJLEDs_Mouth, NUM_MOUTH_LEDS, CRGB::Black);

                int centerRow = MOUTH_ROWS / 2;
                for (int i = 0; i <= rowsToLight; i++) {
                    int rowUp = centerRow - i;
                    int rowDown = centerRow + i;

                    CRGB mouthColor = CRGB::OrangeRed;

                    if (rowUp >= 0 && rowUp < MOUTH_ROWS) {
                        for (int led = 0; led < mouthRowLeds[rowUp]; led++) {
                            DJLEDs_Mouth[mouthRowStart[rowUp] + led] = mouthColor;
                        }
                    }
                    if (rowDown >= 0 && rowDown < MOUTH_ROWS && rowDown != rowUp) {
                        for (int led = 0; led < mouthRowLeds[rowDown]; led++) {
                            DJLEDs_Mouth[mouthRowStart[rowDown] + led] = mouthColor;
                        }
                    }
                }
                FastLED.show();
                if (elapsed > 600) nextPhase();
            }
            break;

        case PHASE_BODY_SWEEP:
            // Sweep through body panels
            {
                uint8_t pos = (elapsed / 40) % (NUM_LEDS_PER_PANEL * 2);

                fadeToBlackBy(DJLEDs_Right, NUM_LEDS_PER_PANEL, 40);
                fadeToBlackBy(DJLEDs_Middle, NUM_LEDS_PER_PANEL, 40);
                fadeToBlackBy(DJLEDs_Left, NUM_LEDS_PER_PANEL, 40);

                if (pos < NUM_LEDS_PER_PANEL) {
                    DJLEDs_Right[pos] = CRGB::OrangeRed;
                    DJLEDs_Middle[pos] = CRGB::OrangeRed;
                    DJLEDs_Left[pos] = CRGB::OrangeRed;
                } else {
                    uint8_t revPos = NUM_LEDS_PER_PANEL - 1 - (pos - NUM_LEDS_PER_PANEL);
                    DJLEDs_Right[revPos] = CRGB::OrangeRed;
                    DJLEDs_Middle[revPos] = CRGB::OrangeRed;
                    DJLEDs_Left[revPos] = CRGB::OrangeRed;
                }
                FastLED.show();
                if (elapsed > 1500) nextPhase();
            }
            break;

        case PHASE_FLASH:
            // Final flash
            if (elapsed < 100) {
                fill_solid(DJLEDs_Right, NUM_LEDS_PER_PANEL, CRGB::White);
                fill_solid(DJLEDs_Middle, NUM_LEDS_PER_PANEL, CRGB::White);
                fill_solid(DJLEDs_Left, NUM_LEDS_PER_PANEL, CRGB::White);
            } else {
                fadeToBlackBy(DJLEDs_Right, NUM_LEDS_PER_PANEL, 30);
                fadeToBlackBy(DJLEDs_Middle, NUM_LEDS_PER_PANEL, 30);
                fadeToBlackBy(DJLEDs_Left, NUM_LEDS_PER_PANEL, 30);
            }
            FastLED.show();
            if (elapsed > 500) nextPhase();
            break;

        case PHASE_COMPLETE:
            complete = true;
            Serial.println(F("Startup sequence complete"));
            break;
    }
}

void StartupSequence::nextPhase() {
    currentPhase = (Phase)(currentPhase + 1);
    phaseStartTime = millis();
}
