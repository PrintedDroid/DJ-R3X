/////////////////////////////////////////////////////////////////////////////////
//
//  Firmware for the Printed Droid DJ Rex Light Panels System
//  www.printed-droid.com
//
//  ==============================================================================
//
//  VERSION: 5.0.0 - "Enhanced Edition"
//  DATE:    2025/11/30
//  BASE:    v3.1 "Playlist Edition" with v4.2 enhancements
//
//  ==============================================================================
//
//  HARDWARE:
//  - Controller: LOLIN C3 Mini (ESP32-C3)
//  - LEDs:       WS2812B (GRB color order)
//  - Serial Baud Rate: 115200
//
//  LED LAYOUT:
//  - Body Panels:  3 panels with 20 LEDs each (total 60 LEDs)
//                  - 8 side LEDs per panel
//                  - 3x4 block LEDs per panel
//  - Eyes:         2 LEDs
//  - Mouth:        80 LEDs in a 12-row matrix
//
//  PINOUT & RMT CHANNELS:
//  - RMT CH1 (Pin 3, 4, 5):  Body Panels (Right, Middle, Left)
//  - RMT CH2 (Pin 6):        Mouth and Eyes (daisy-chained)
//  - MIC_PIN (Pin 1):        Analog microphone input for audio reactivity
//
//  ==============================================================================
//
//  KEY FEATURES (from v3.1):
//
//  --- Modular LED Control ---
//  * Controls a total of 142 LEDs across 5 independent outputs
//  * Independent animation arrays for all panels, eyes, and mouth
//
//  --- Body Animation System ---
//  * 17 distinct patterns with configurable colors and speeds
//  * Advanced "Random Blocks" pattern with per-block color assignment
//
//  --- Eye Animation System ---
//  * Multi-Color Modes: Single, Dual Color, Alternating
//  * Natural Flicker Effect with configurable timing
//
//  --- Mouth Animation System ---
//  * 12 animation patterns with Dual Color Split Modes
//  * Brightness Compensation for even light distribution
//
//  --- Audio Reactivity Engine ---
//  * Auto-Gain for consistent visual response
//  * Selectable audio routing (mouth, body, or all)
//
//  --- Playlist and Transition System ---
//  * Custom pattern sequences with smooth crossfade transitions
//
//  --- Configuration & Persistence ---
//  * 3 User Preset Slots with persistent memory
//
//  ==============================================================================
//
//  NEW IN v5.0 (from v4.2):
//
//  --- Thread Safety (FreeRTOS) ---
//  * LED Mutex for safe multi-threading operations
//  * Separate Audio Task on Core 0
//
//  --- System Monitoring ---
//  * Health Check System with automatic error detection
//  * Emergency Mode with automatic restart
//  * Memory Monitoring with low-memory warnings
//  * Performance Monitoring with loop counter
//
//  --- New Modules ---
//  * Event Logger: System event logging
//  * Pattern Manager: Safe pattern transitions
//  * Preset Manager: 10 preset slots (up from 3)
//  * Startup Sequence: Enhanced boot animation
//
//  --- Extended Patterns ---
//  * 20 Body Patterns (3 new: Plasma, Fire, Twinkle)
//  * 15 Mouth Patterns (3 new: Matrix, Heartbeat, Spectrum)
//
//  ==============================================================================
//  FastLED Library Version: 3.9.0 required!!!
//
/////////////////////////////////////////////////////////////////////////////////


#include <FastLED.h>
#include <Preferences.h>
#include "config.h"
#include "globals.h"
#include "patterns_body.h"
#include "patterns_mouth.h"
#include "eyes.h"
#include "helpers.h"
#include "serial_commands.h"
#include "settings.h"
#include "audio.h"
#include "demo.h"

// Combined LED array for Eyes + Mouth
CRGB eyesMouthLEDs[NUM_EYES + NUM_MOUTH_LEDS];

void setup() {
    Serial.begin(115200);
    delay(1000);
    while (!Serial) {
        delay(10);
    }
    
    Serial.println(F("Starting..."));
    Serial.flush();

    Serial.println(F("Printed-Droid DJ Rex v5.0.0 - Enhanced Edition"));
    Serial.println(F("==============================================="));
    Serial.println(F("ESP32-C3 Mini | Base: v3.1 + v4.2 Features"));
    
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    
    initSettings();
    
    FastLED.addLeds<LED_TYPE, LED_PIN_RIGHT, COLOR_ORDER>(DJLEDs_Right, NUM_LEDS_PER_PANEL);
    FastLED.addLeds<LED_TYPE, LED_PIN_MIDDLE, COLOR_ORDER>(DJLEDs_Middle, NUM_LEDS_PER_PANEL);
    FastLED.addLeds<LED_TYPE, LED_PIN_LEFT, COLOR_ORDER>(DJLEDs_Left, NUM_LEDS_PER_PANEL);
    FastLED.addLeds<LED_TYPE, EYES_MOUTH_PIN, COLOR_ORDER>(eyesMouthLEDs, NUM_EYES + NUM_MOUTH_LEDS);
    
    FastLED.setBrightness(ledBrightness);
    
    FastLED.clear();
    FastLED.show();

    initializeHelpers();
    initializeEyes();
    initializeAudio();
    
    Serial.println(F("System ready! Type 'help' for commands."));
    printCurrentSettings();
}

void mapEyesMouthArrays() {
    memcpy(&eyesMouthLEDs[0], DJLEDs_Eyes, NUM_EYES * sizeof(CRGB));
    memcpy(&eyesMouthLEDs[NUM_EYES], DJLEDs_Mouth, NUM_MOUTH_LEDS * sizeof(CRGB));
}

// Function to start a transition
void startTransition(uint8_t newPattern) {
    if (newPattern == currentPattern) return; // Don't transition to the same pattern

    // 1. Copy the current, final LED state to the "old" arrays
    memcpy(old_DJLEDs_Right, DJLEDs_Right, sizeof(DJLEDs_Right));
    memcpy(old_DJLEDs_Middle, DJLEDs_Middle, sizeof(DJLEDs_Middle));
    memcpy(old_DJLEDs_Left, DJLEDs_Left, sizeof(DJLEDs_Left));
    memcpy(old_DJLEDs_Eyes, DJLEDs_Eyes, sizeof(DJLEDs_Eyes));
    memcpy(old_DJLEDs_Mouth, DJLEDs_Mouth, sizeof(DJLEDs_Mouth));

    // 2. Set the new pattern
    currentPattern = newPattern;

    // 3. Start the transition timer
    transitionActive = true;
    transitionStartTime = millis();
}

// Function that handles the blending logic during a transition
void handleTransition() {
    if (!transitionActive) {
        return; // Nothing to do
    }

    unsigned long elapsed = millis() - transitionStartTime;

    if (elapsed >= transitionDuration) {
        transitionActive = false; // Transition is over
        return;
    }

    // Calculate how far along the blend is (0-255)
    uint8_t blendAmount = map(elapsed, 0, transitionDuration, 0, 255);

    // Blend each LED array
    // The "new" pattern has already been calculated and is in the main DJLEDs arrays.
    // We blend the saved "old" state into the "new" state.
    blend(old_DJLEDs_Right, DJLEDs_Right, DJLEDs_Right, NUM_LEDS_PER_PANEL, blendAmount);
    blend(old_DJLEDs_Middle, DJLEDs_Middle, DJLEDs_Middle, NUM_LEDS_PER_PANEL, blendAmount);
    blend(old_DJLEDs_Left, DJLEDs_Left, DJLEDs_Left, NUM_LEDS_PER_PANEL, blendAmount);
    blend(old_DJLEDs_Eyes, DJLEDs_Eyes, DJLEDs_Eyes, NUM_EYES, blendAmount);
    blend(old_DJLEDs_Mouth, DJLEDs_Mouth, DJLEDs_Mouth, NUM_MOUTH_LEDS, blendAmount);
}

void handlePlaylist() {
    if (!playlistActive || playlistSize == 0) {
        return;
    }

    if (millis() - playlistPatternStartTime >= (playlist[playlistIndex].duration * 1000UL)) {
        playlistIndex++;
        if (playlistIndex >= playlistSize) {
            playlistIndex = 0;
        }

        // Start a transition instead of changing the pattern directly
        startTransition(playlist[playlistIndex].pattern);
        playlistPatternStartTime = millis();

        Serial.print(F("Playlist: Transitioning to pattern "));
        Serial.println(playlist[playlistIndex].pattern);
    }
}

void loop() {
    handlePlaylist();

    // Check for manual pattern change requests
    if (requestedPattern != -1) {
        startTransition(requestedPattern);
        requestedPattern = -1; // Reset request
    }
    
    if (checkSerialCommand()) {
        processSerialCommand();
    }
    
    if (demoMode) {
        handleDemoMode();
    }
    
    // Always run the current pattern logic
    gPatterns[currentPattern]();
    
    if (currentPattern != 0) {
        updateEyes();
    }
    
    if (mouthEnabled && currentPattern != 0) {
        updateMouth();
    }

    // After calculating the new pattern, apply the transition blend if active
    handleTransition();
    
    static unsigned long LEDUpdateMillis = 0;
    if (millis() - LEDUpdateMillis > 20) {
        LEDUpdateMillis = millis();
        mapEyesMouthArrays();
        FastLED.show();
    }
    
    EVERY_N_MILLISECONDS(20) {
        gHue++;
    }
}