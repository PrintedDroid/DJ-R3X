#ifndef GLOBALS_H
#define GLOBALS_H

#include <FastLED.h>
#include <Preferences.h>
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
extern uint8_t beatsPerMinute;
extern uint8_t fadeSpeed;
extern uint8_t solidColorIndex;
extern uint8_t confettiColor1;
extern uint8_t confettiColor2;
extern uint8_t eyeColorIndex;
extern uint8_t solidMode;

// Brightness controls
extern uint8_t eyeBrightness;
extern uint8_t bodyBrightness;
extern uint8_t mouthOuterBoost;
extern uint8_t mouthInnerBoost;

// Eye control variables
extern uint8_t eyeColorIndex2; // NEU
extern uint8_t eyeMode;        // NEU

// Eye flicker control variables
extern bool eyeFlickerEnabled;
extern uint16_t eyeFlickerMinTime;
extern uint16_t eyeFlickerMaxTime;
extern uint8_t eyeStaticBrightness;

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
extern uint8_t matrixDrops[3][8];
extern uint8_t matrixBright[3][8];

// Mouth-specific parameters
extern uint8_t mouthPattern;
extern uint8_t mouthColorIndex;
extern uint8_t mouthColorIndex2; // NEU
extern uint8_t mouthSplitMode;   // NEU
extern uint8_t mouthBrightness;
extern bool mouthEnabled;
extern uint8_t talkSpeed;
extern uint8_t smileWidth;
extern uint8_t waveSpeed;        // NEU
extern uint8_t pulseSpeed;       // NEU

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

// User preset structure
struct UserPreset {
    uint8_t pattern;
    uint8_t brightness;
    uint8_t speed;
    uint8_t solidColor;
    uint8_t eyeColor;
    uint8_t mouthPattern;
    uint8_t mouthColor;
    uint8_t audioMode;
    uint8_t blockColors[9];
    uint8_t sideColors[3];
    uint8_t sideMode;
};

// Audio processing
// v5.0.1: Marked as volatile for thread-safety (FreeRTOS audio task on S3)
extern volatile int audioLevel;
extern volatile int audioThreshold;
extern unsigned long lastAudioRead;
extern volatile int audioSamples[10];
extern volatile int averageAudio;
extern volatile uint8_t audioSampleIdx;
extern uint8_t audioMode;
extern uint8_t audioSensitivity;
extern bool audioAutoGain;
extern volatile int audioMinLevel;
extern volatile int audioMaxLevel;

// v5.1: Audio input mode (Microphone or Line-In)
extern uint8_t audioInputMode;

// Timing arrays
extern uint16_t IntervalTime[TOTAL_BODY_LEDS];
extern unsigned long LEDMillis[TOTAL_BODY_LEDS];
extern bool LEDOn[TOTAL_BODY_LEDS];

// LED arrays
extern CRGB DJLEDs_Right[NUM_LEDS_PER_PANEL];
extern CRGB DJLEDs_Middle[NUM_LEDS_PER_PANEL];
extern CRGB DJLEDs_Left[NUM_LEDS_PER_PANEL];
extern CRGB DJLEDs_Eyes[NUM_EYES];
extern CRGB DJLEDs_Mouth[NUM_MOUTH_LEDS];

// ...
// LED arrays
extern CRGB DJLEDs_Right[NUM_LEDS_PER_PANEL];
extern CRGB DJLEDs_Middle[NUM_LEDS_PER_PANEL];
extern CRGB DJLEDs_Left[NUM_LEDS_PER_PANEL];
extern CRGB DJLEDs_Eyes[NUM_EYES];
extern CRGB DJLEDs_Mouth[NUM_MOUTH_LEDS];

//Arrays to store the state of the old pattern for transitions
extern CRGB old_DJLEDs_Right[NUM_LEDS_PER_PANEL];
extern CRGB old_DJLEDs_Middle[NUM_LEDS_PER_PANEL];
extern CRGB old_DJLEDs_Left[NUM_LEDS_PER_PANEL];
extern CRGB old_DJLEDs_Eyes[NUM_EYES];
extern CRGB old_DJLEDs_Mouth[NUM_MOUTH_LEDS];

//Transition control variables
extern bool transitionActive;
extern unsigned long transitionStartTime;
extern int8_t requestedPattern; // Used to trigger a transition from serial commands
const uint16_t transitionDuration = 1000; // Define const here to make it visible everywhere

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
extern const char* EyeModeNames[3];
extern const char* MouthSplitNames[5];
extern const char* AudioModeNames[5];
extern const char* AudioInputModeNames[2];  // v5.1
extern const char* patternNames[NUM_PATTERNS];


// Playlist
struct PlaylistEntry {
    uint8_t pattern;
    uint16_t duration; // in Sekunden
};

extern PlaylistEntry playlist[10];
extern uint8_t playlistSize;
extern bool playlistActive;
extern uint8_t playlistIndex;
extern unsigned long playlistPatternStartTime;

// Pattern function list
typedef void (*SimplePatternList[])();
extern SimplePatternList gPatterns;

// v5.0: Startup sequence control
extern bool startupSequenceEnabled;

#endif