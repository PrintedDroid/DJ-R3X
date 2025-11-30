#ifndef GLOBALS_H
#define GLOBALS_H

// System includes first
#include <Arduino.h>
#include <Preferences.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Project config
#include "config.h"

// Settings storage
extern Preferences preferences;

// Pattern parameters
extern uint8_t currentPattern;
extern uint8_t ledBrightness;
extern uint8_t effectSpeed;
extern uint16_t sideMinTime;
extern uint16_t sideMaxTime;
extern uint16_t blockMinTime;
extern uint16_t blockMaxTime;
extern uint8_t fadeSpeed;
extern uint8_t solidColorIndex;
extern uint8_t confettiColor1;
extern uint8_t confettiColor2;
extern uint8_t eyeColorIndex;
extern uint8_t eyeColorIndex2;
extern uint8_t eyeMode;
extern uint8_t solidMode;

// Eye flicker control
extern bool eyeFlickerEnabled;
extern uint16_t eyeFlickerMinTime;
extern uint16_t eyeFlickerMaxTime;
extern uint8_t eyeStaticBrightness;

// Brightness controls
extern uint8_t eyeBrightness;
extern uint8_t bodyBrightness;
extern uint8_t mouthOuterBoost;
extern uint8_t mouthInnerBoost;

// Block-specific colors
extern uint8_t blockColors[9];

// Side LED color settings
extern uint8_t sideColor1;
extern uint8_t sideColor2;
extern uint8_t sideColor3;
extern uint8_t sideColorMode;
extern uint8_t sideBlinkRate;
extern uint8_t blockBlinkRate;
extern uint8_t sideColorCycleIndex;

// Pattern-specific parameters
extern uint8_t knightColorIndex;
extern uint8_t breathingColorIndex;
extern uint8_t matrixColorIndex;
extern uint8_t strobeColorIndex;
extern uint8_t knightPos;
extern bool knightDir;
extern uint8_t breathingBright;
extern bool breathingUp;
extern bool strobeState;
extern unsigned long knightMillis;
extern unsigned long breathingMillis;
extern unsigned long matrixMillis;
extern unsigned long strobeMillis;
extern uint16_t knightInterval;
extern uint16_t strobeInterval;
extern uint8_t matrixDrops[3][SIDE_LEDS_COUNT];
extern uint8_t matrixBright[3][SIDE_LEDS_COUNT];

// Mouth-specific parameters
extern uint8_t mouthPattern;
extern uint8_t mouthColorIndex;
extern uint8_t mouthColorIndex2;
extern uint8_t mouthSplitMode;
extern uint8_t mouthBrightness;
extern bool mouthEnabled;
extern uint8_t talkSpeed;
extern uint8_t smileWidth;
extern uint8_t waveSpeed;
extern uint8_t pulseSpeed;

// Solid Flash pattern variables
extern uint8_t flashColorIndex;
extern uint8_t flashSpeed;
extern bool flashState;
extern unsigned long lastFlashTime;

// Short Circuit pattern variables
extern uint8_t shortColorIndex;

// Demo mode variables
extern bool demoMode;
extern uint16_t demoTime;
extern unsigned long lastDemoChange;
extern uint8_t demoPatternIndex;
extern uint8_t demoColorIndex;
extern uint8_t demoStep;

// Startup sequence variables
extern bool startupComplete;
extern unsigned long startupBeginTime;
extern uint8_t startupPhase;
extern bool panelActive[3];
extern unsigned long panelActivationTime[3];

// Pattern properties for demo mode
struct PatternProperties {
    bool hasColorVariations;
    uint8_t* colorIndexPtr;
    const char* colorName;
};
extern const PatternProperties patternProps[NUM_PATTERNS];

// Audio processing
extern int audioThreshold;
extern uint8_t audioMode;
extern uint8_t audioSensitivity;
extern bool audioAutoGain;

// Timing arrays
extern uint16_t IntervalTime[TOTAL_BODY_LEDS];
extern unsigned long LEDMillis[TOTAL_BODY_LEDS];
extern bool LEDOn[TOTAL_BODY_LEDS];

// LED arrays for ESP32-C3 configuration
// Body panels chained on IO3, Eyes on IO6, Mouth on IO7 (all separate)
extern CRGB bodyLEDsChained[TOTAL_BODY_LEDS];   // Right->Middle->Left on IO3
extern CRGB DJLEDs_Eyes[NUM_EYES];              // Eyes on IO6 (separate)
extern CRGB DJLEDs_Mouth[NUM_MOUTH_LEDS];       // Mouth on IO7 (separate)

// Virtual LED array pointers for body panels (for code compatibility)
extern CRGB* DJLEDs_Right;    // Points to bodyLEDsChained[0]  (LEDs 0-19)
extern CRGB* DJLEDs_Middle;   // Points to bodyLEDsChained[20] (LEDs 20-39)
extern CRGB* DJLEDs_Left;     // Points to bodyLEDsChained[40] (LEDs 40-59)

// Eyes variables
extern uint16_t EyesIntervalTime[NUM_EYES];
extern unsigned long EyesLEDMillis[NUM_EYES];
extern bool EyesLEDOn[NUM_EYES];
extern uint8_t EyesLEDBrightness[NUM_EYES];
extern uint8_t EyesLEDMinBrightness[NUM_EYES];

// Animation variables
extern uint8_t gHue;
extern uint8_t gSat;
extern bool updown;
extern unsigned long FadeMillis;
extern uint16_t DecayTime;
extern uint16_t FadeInterval;

// Mouth constants
extern const uint8_t mouthRowLeds[MOUTH_ROWS];
extern const uint8_t mouthRowStart[MOUTH_ROWS];

// Color constants
extern const CRGB StandardColors[NUM_STANDARD_COLORS];
extern const char* ColorNames[NUM_STANDARD_COLORS];
extern const char* SideColorModeNames[5];
extern const char* MouthPatternNames[NUM_MOUTH_PATTERNS];
extern const char* MouthSplitModeNames[5];
extern const char* AudioModeNames[6];
extern const char* EyeModeNames[5];
extern const char* patternNames[NUM_PATTERNS];

// Pattern function list
typedef void (*SimplePatternList[])();
extern SimplePatternList gPatterns;

// Thread synchronization
extern SemaphoreHandle_t ledMutex;
extern volatile bool patternUpdating;

// Demo mode state preservation
extern uint8_t savedPattern;
extern bool savedDemoState;

// Smooth color transitions
extern uint8_t colorFadeSteps;
extern bool enableSmoothTransitions;

// System state
extern bool systemReady;
extern unsigned long bootTime;

// Serial command globals
extern String inputString;
extern boolean stringComplete;

extern uint32_t failedCommands;

// Hardware validation function
void validateHardwareConfiguration();

#endif