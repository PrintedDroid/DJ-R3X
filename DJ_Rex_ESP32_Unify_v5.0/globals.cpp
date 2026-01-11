#include "globals.h"

// Initialize all global variables
Preferences preferences;

// Pattern parameters
uint8_t currentPattern = 16;
uint8_t ledBrightness = 90;
uint8_t effectSpeed = 128;
uint16_t sideMinTime = 500;
uint16_t sideMaxTime = 2500;
uint16_t blockMinTime = 200;
uint16_t blockMaxTime = 1500;
uint8_t beatsPerMinute = 62;
uint8_t fadeSpeed = 8;
uint8_t solidColorIndex = 0;
uint8_t confettiColor1 = 0;
uint8_t confettiColor2 = 3;
uint8_t eyeColorIndex = 14;
uint8_t solidMode = 0;

// Brightness controls
uint8_t eyeBrightness = 125;
uint8_t bodyBrightness = 100;
uint8_t mouthOuterBoost = 100;
uint8_t mouthInnerBoost = 150;

// Eye control variables
uint8_t eyeColorIndex2 = 12;
uint8_t eyeMode = 0;

// Eye flicker control variables
bool eyeFlickerEnabled = false;
uint16_t eyeFlickerMinTime = 200;
uint16_t eyeFlickerMaxTime = 1600;
uint8_t eyeStaticBrightness = 255;

// Block colors
uint8_t blockColors[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};

// Side LED settings
uint8_t sideColor1 = 0;
uint8_t sideColor2 = 2;
uint8_t sideColor3 = 3;
uint8_t sideColorMode = 0;
uint8_t sideBlinkRate = 128;
uint8_t blockBlinkRate = 128;
uint8_t sideColorCycleIndex = 0;

// Pattern-specific
uint8_t knightColorIndex = 0;
uint8_t breathingColorIndex = 2;
uint8_t matrixColorIndex = 11;
uint8_t strobeColorIndex = 3;
uint8_t knightPos = 0;
bool knightDir = true;
uint8_t breathingBright = 0;
bool breathingUp = true;
bool strobeState = false;
unsigned long knightMillis = 0;
unsigned long breathingMillis = 0;
unsigned long matrixMillis = 0;
unsigned long strobeMillis = 0;
uint16_t knightInterval = 100;
uint16_t strobeInterval = 100;
uint8_t matrixDrops[3][8];
uint8_t matrixBright[3][8];

// Mouth parameters
uint8_t mouthPattern = 1;
uint8_t mouthColorIndex = 0;
uint8_t mouthColorIndex2 = 3;
uint8_t mouthSplitMode = 0;
uint8_t mouthBrightness = 90;
bool mouthEnabled = true;
uint8_t talkSpeed = 5;
uint8_t smileWidth = 6;
uint8_t waveSpeed = 5;
uint8_t pulseSpeed = 5;

// Flash pattern
uint8_t flashColorIndex = 0;
uint8_t flashSpeed = 5;
bool flashState = false;
unsigned long lastFlashTime = 0;

// Short Circuit
uint8_t shortColorIndex = 3;

// Demo mode
bool demoMode = false;
uint16_t demoTime = 10;
unsigned long lastDemoChange = 0;
uint8_t demoPatternIndex = 1;
uint8_t demoColorIndex = 0;
uint8_t demoStep = 0;

// Audio
// v5.0.1: Marked as volatile for thread-safety (FreeRTOS audio task on S3)
volatile int audioLevel = 0;
volatile int audioThreshold = 100;
unsigned long lastAudioRead = 0;
volatile int audioSamples[10] = {0};
volatile int averageAudio = 0;
volatile uint8_t audioSampleIdx = 0;
uint8_t audioMode = AUDIO_ALL;
uint8_t audioSensitivity = 5;
bool audioAutoGain = true;
volatile int audioMinLevel = 4095;
volatile int audioMaxLevel = 0;

// Timing arrays
uint16_t IntervalTime[TOTAL_BODY_LEDS];
unsigned long LEDMillis[TOTAL_BODY_LEDS];
bool LEDOn[TOTAL_BODY_LEDS];

// LED arrays
CRGB DJLEDs_Right[NUM_LEDS_PER_PANEL];
CRGB DJLEDs_Middle[NUM_LEDS_PER_PANEL];
CRGB DJLEDs_Left[NUM_LEDS_PER_PANEL];
CRGB DJLEDs_Eyes[NUM_EYES];
CRGB DJLEDs_Mouth[NUM_MOUTH_LEDS];

//Arrays for transition state
CRGB old_DJLEDs_Right[NUM_LEDS_PER_PANEL];
CRGB old_DJLEDs_Middle[NUM_LEDS_PER_PANEL];
CRGB old_DJLEDs_Left[NUM_LEDS_PER_PANEL];
CRGB old_DJLEDs_Eyes[NUM_EYES];
CRGB old_DJLEDs_Mouth[NUM_MOUTH_LEDS];

//Transition control variables
bool transitionActive = false;
unsigned long transitionStartTime = 0;
int8_t requestedPattern = -1; // -1 means no request

// Eyes variables
uint16_t EyesIntervalTime[NUM_EYES];
unsigned long EyesLEDMillis[NUM_EYES];
bool EyesLEDOn[NUM_EYES];
uint8_t EyesLEDBrightness[NUM_EYES];
uint8_t EyesLEDMinBrightness[NUM_EYES];

// Animation variables
uint8_t gHue = 0;
uint8_t gSat = 0;
bool updown = 0;
unsigned long FadeMillis = 0;
uint16_t DecayTime = DECAYTIME;
uint16_t FadeInterval = 0;

// Playlist-Definition und Initialisierung
PlaylistEntry playlist[10] = {
    {5, 15},  // Muster 5 (Rainbow) for 15s
    {12, 20}, // Muster 12 (Breathing) for 20s
    {7, 15},  // Muster 7 (Confetti) for 15s
    {11, 20}  // Muster 11 (Knight Rider) for 20s
};
uint8_t playlistSize = 4; // Wir haben 4 Einträge in unserer Standard-Playlist
bool playlistActive = false; // Playlist ist standardmäßig aus
uint8_t playlistIndex = 0;
unsigned long playlistPatternStartTime = 0;

// Mouth layout
const uint8_t mouthRowLeds[MOUTH_ROWS] = {8, 8, 8, 8, 8, 8, 8, 8, 6, 4, 4, 2};
const uint8_t mouthRowStart[MOUTH_ROWS] = {0, 8, 16, 24, 32, 40, 48, 56, 64, 70, 74, 78};

// Extended color palette
const CRGB StandardColors[NUM_STANDARD_COLORS] = {
    CRGB::Red, CRGB::Green, CRGB::Blue, CRGB(255, 230, 240), CRGB(255, 180, 0), CRGB::Cyan, CRGB::Magenta,
    CRGB(255, 140, 0), CRGB::Purple, CRGB(255, 20, 147), CRGB::Black, CRGB(0, 200, 100), CRGB(100, 150, 255),
    CRGB(140, 0, 255), CRGB(255, 120, 0), CRGB(255, 255, 255), CRGB(120, 255, 0), CRGB(0, 128, 128),
    CRGB(255, 255, 255), CRGB(180, 200, 255)
};

const char* ColorNames[NUM_STANDARD_COLORS] = {
    "Red", "Green", "Blue", "Warm White", "Yellow", "Cyan", "Magenta", "Orange", "Purple", "Pink", "Black",
    "Matrix Green", "Ice Blue", "UV Purple", "Amber", "Cool White", "Lime Green", "Teal", "Pure White", "Blue White"
};

const char* SideColorModeNames[5] = {
    "Random from 3", "Cycle through 3", "Color 1 only", "Color 2 only", "Color 3 only"
};

const char* MouthPatternNames[NUM_MOUTH_PATTERNS] = {
    "Off", "Talk", "Smile", "Audio Reactive", "Rainbow", "Debug",
    "Wave", "Pulse", "VU Meter Horiz", "VU Meter Vert", "Frown", "Sparkle",
    "Matrix", "Heartbeat", "Spectrum"  // v5.0 new patterns
};

const char* EyeModeNames[3] = {
    "Single Color", "Dual Color", "Alternating"
};

const char* MouthSplitNames[5] = {
    "Off", "Vertical", "Horizontal", "Inner/Outer", "Random"
};

const char* AudioModeNames[5] = {
    "Off", "Mouth Only", "Body Sides Only", "Body All", "Everything"
};

const char* patternNames[NUM_PATTERNS] = {
    "LEDs Off", "Random Blocks", "Solid Color", "Short Circuit", "Confetti Red/White", "Rainbow", "Rainbow with Glitter",
    "Confetti", "Juggle", "Audio Sync", "Solid Flash", "Knight Rider", "Breathing", "Matrix Rain", "Strobe",
    "Audio VU Meter", "Custom Block Sequence",
    "Plasma", "Fire", "Twinkle"  // v5.0 new patterns
};

// v5.0: Startup sequence control
bool startupSequenceEnabled = STARTUP_SEQUENCE_ENABLED;