/////////////////////////////////////////////////////////////////////////////////
//
//  Firmware for the Printed Droid DJ Rex Light Panels System
//  www.printed-droid.com
//
//  ==============================================================================
//
//  VERSION: 3.1 - "Playlist Edition"
//  DATE:    2025/09/17
//
//  ==============================================================================
//
//  HARDWARE:
//  - Controller: LOLIN S3 Mini (ESP32-C3)
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
//  KEY FEATURES:
//
//  --- Modular LED Control ---
//  * Controls a total of 142 LEDs across 5 independent outputs.
//  * Hybrid Parallel Output: Utilizes 2 hardware RMT channels for high-performance
//      control of two body panels and 2 software-driven "bit-banging" channels
//      for the third panel and the mouth/eye chain.
//  * Independent animation arrays for Left, Middle, and Right body panels,
//      plus separate arrays for Eyes and Mouth for clear and logical code.
//
//  --- Body Animation System ---
//  * 17 distinct patterns, including generative effects (Rainbow, Confetti, Matrix),
//      classic animations (Knight Rider, Breathing), and audio-reactive modes.
//  * Highly Configurable Patterns: Most animations allow for custom colors and speeds.
//  * Advanced "Random Blocks" pattern with per-block color assignment and
//      separate color modes and blink rates for side LEDs.
//
//  --- Eye Animation System ---
//  * Multi-Color Modes: Eyes can be set to a single shared color, two
//      independent colors (Dual Color), or alternating between two colors.
//  * Natural Flicker Effect: A configurable flicker with randomized timing to
//      simulate a realistic, non-mechanical effect.
//  * Dedicated brightness control (eyebrightness) and a separate static
//      brightness setting for when the flicker is disabled.
//
//  --- Mouth Animation System ---
//  * 12 animation patterns, including expressive (Smile, Frown, Talk),
//      audio-driven (VU Meters), and abstract visualizers (Wave, Pulse, Sparkle).
//  * Dual Color Split Modes: A powerful feature to split two user-defined colors
//      across the mouth LEDs, either vertically, horizontally, by inner/outer
//      position, or randomly.
//  * Brightness Compensation: Fine-tuned brightness boosts for outer vs. inner
//      LEDs to ensure even light distribution through a physical diffuser.
//
//  --- Audio Reactivity Engine ---
//  * Driven by an analog microphone connected to a dedicated ADC pin.
//  * Advanced Audio Routing: The audio signal can be selectively routed to control
//      the mouth only, the body LEDs, or everything simultaneously.
//  * Automatic Gain Control (Auto-Gain): Automatically adjusts sensitivity to
//      loud or quiet music for a consistent visual response.
//
//  --- Playlist and Transition System ---
//  * Playlist Mode: Users can create custom sequences of patterns, each with a
//      specific duration, via a simple serial command.
//  * Smooth Crossfade Transitions: When a pattern changes (manually or via
//      the playlist), the system performs a 1-second smooth fade from the old
//      pattern to the new, eliminating hard visual cuts.
//
//  --- Configuration & Persistence ---
//  * Full control of all parameters via a comprehensive serial command interface.
//  * Persistent Memory: All settings are saved to the ESP32's non-volatile
//      flash memory, automatically reloading on startup.
//  * 3 User Preset Slots: Users can save and load their favorite complete
//      configurations on the fly.
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
    
    Serial.println(F("Printed-Droid DJ Rex v3.1 - Complete Edition"));
    Serial.println(F("============================================="));
    Serial.println(F("ESP32-C3 Mini with Mouth & Playlist Support"));
    
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