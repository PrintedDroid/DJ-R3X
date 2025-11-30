#include "globals.h"

// Initialize all global variables
Preferences preferences;

// Pattern parameters
uint8_t currentPattern = 16;  // Default to Custom Block Sequence
uint8_t ledBrightness = 90;
uint8_t effectSpeed = 128;
uint16_t sideMinTime = 500;
uint16_t sideMaxTime = 2500;
uint16_t blockMinTime = 200;
uint16_t blockMaxTime = 1500;
uint8_t fadeSpeed = 8;
uint8_t solidColorIndex = 0;
uint8_t confettiColor1 = 0;
uint8_t confettiColor2 = 3;
uint8_t eyeColorIndex = 3;  // White
uint8_t eyeColorIndex2 = 2; // Blue
uint8_t eyeMode = EYE_MODE_SINGLE;
uint8_t solidMode = 0;

// Eye flicker control
bool eyeFlickerEnabled = true;
uint16_t eyeFlickerMinTime = 200;
uint16_t eyeFlickerMaxTime = 1600;
uint8_t eyeStaticBrightness = 255;

// Brightness controls
uint8_t eyeBrightness = 125;
uint8_t bodyBrightness = 100;
uint8_t mouthOuterBoost = 100;
uint8_t mouthInnerBoost = 150;

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
uint8_t matrixDrops[3][SIDE_LEDS_COUNT];
uint8_t matrixBright[3][SIDE_LEDS_COUNT];

// Mouth parameters
uint8_t mouthPattern = 1;
uint8_t mouthColorIndex = 2;
uint8_t mouthColorIndex2 = 0;
uint8_t mouthSplitMode = MOUTH_SPLIT_OFF;
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

// Startup sequence
bool startupComplete = false;
unsigned long startupBeginTime = 0;
uint8_t startupPhase = 0;
bool panelActive[3] = {false, false, false};
unsigned long panelActivationTime[3] = {0, 0, 0};

// Audio
int audioThreshold = 100;
uint8_t audioMode = AUDIO_ALL;
uint8_t audioSensitivity = 5;
bool audioAutoGain = true;

// Timing arrays
uint16_t IntervalTime[TOTAL_BODY_LEDS];
unsigned long LEDMillis[TOTAL_BODY_LEDS];
bool LEDOn[TOTAL_BODY_LEDS];

// CORRECTED LED arrays for ESP32-C3 chained configuration
// Physical LED strips (actual hardware connections)
CRGB bodyLEDsChained[TOTAL_BODY_LEDS];           // Right->Middle->Left on IO3
CRGB eyesMouthLEDs[NUM_EYES + NUM_MOUTH_LEDS];   // Mouth->Eyes on IO7

// Virtual LED array pointers (for code compatibility)
// These point to sections within the physical chained arrays
CRGB* DJLEDs_Right = &bodyLEDsChained[0];                          // LEDs 0-19
CRGB* DJLEDs_Middle = &bodyLEDsChained[NUM_LEDS_PER_PANEL];        // LEDs 20-39
CRGB* DJLEDs_Left = &bodyLEDsChained[NUM_LEDS_PER_PANEL * 2];      // LEDs 40-59

// Eyes and Mouth within chained strip (Mouth->Eyes configuration on IO7)
CRGB* DJLEDs_Mouth = &eyesMouthLEDs[0];                    // LEDs 0-79 (Mouth first)
CRGB* DJLEDs_Eyes = &eyesMouthLEDs[NUM_MOUTH_LEDS];        // LEDs 80-81 (Eyes follow)

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

// Mouth layout (corrected for Mouth->Eyes chain)
const uint8_t mouthRowLeds[MOUTH_ROWS] = {8, 8, 8, 8, 8, 8, 8, 8, 6, 4, 4, 2};
const uint8_t mouthRowStart[MOUTH_ROWS] = {0, 8, 16, 24, 32, 40, 48, 56, 64, 70, 74, 78};

// Extended color palette
const CRGB StandardColors[NUM_STANDARD_COLORS] = { 
    CRGB::Red,              // 0: Red
    CRGB::Green,            // 1: Green
    CRGB::Blue,             // 2: Blue
    CRGB(255, 253, 240),    // 3: Warm White
    CRGB::Yellow,           // 4: Yellow
    CRGB::Cyan,             // 5: Cyan
    CRGB::Magenta,          // 6: Magenta
    CRGB::Orange,           // 7: Orange
    CRGB::Purple,           // 8: Purple
    CRGB::Pink,             // 9: Pink
    CRGB::Black,            // 10: Black/Random
    CRGB(60, 255, 60),      // 11: Matrix Green
    CRGB(170, 220, 255),    // 12: Ice Blue
    CRGB(140, 0, 255),      // 13: UV Purple
    CRGB(255, 191, 0),      // 14: Amber
    CRGB(255, 255, 255),    // 15: Cool White
    CRGB(180, 255, 0),      // 16: Lime Green
    CRGB(0, 128, 128),      // 17: Teal
    CRGB(180, 180, 180),    // 18: Pure White
    CRGB(200, 220, 255)     // 19: Blue White
};

const char* ColorNames[NUM_STANDARD_COLORS] = {
    "Red", "Green", "Blue", "Warm White", "Yellow",
    "Cyan", "Magenta", "Orange", "Purple", "Pink",
    "Random", "Matrix Green", "Ice Blue", "UV Purple", "Amber",
    "Cool White", "Lime Green", "Teal", "Pure White", "Blue White"
};

const char* SideColorModeNames[5] = {
    "Random from 3", "Cycle through 3", "Color 1 only", "Color 2 only", "Color 3 only"
};

const char* MouthPatternNames[NUM_MOUTH_PATTERNS] = {
    "Off", "Talk", "Smile", "Audio Reactive", "Rainbow",
    "Debug", "Wave", "Pulse", "Sparkle", "Scanline",
    "Firework", "Audio VU", "Matrix", "Heartbeat", "Spectrum"
};

const char* MouthSplitModeNames[5] = {
    "Single Color", "Left/Right", "Alternating Rows", "Center Gradient", "Top/Bottom"
};

const char* AudioModeNames[6] = {
    "Off", "Mouth Only", "Body Sides Only", "Body All", "Everything", "Frequency Bands"
};

const char* EyeModeNames[5] = {
    "Single Color", "Dual Colors", "Alternating", "Rainbow", "Audio Reactive"
};

const char* patternNames[NUM_PATTERNS] = {
    "LEDs Off",              // 0
    "Random Blocks",         // 1
    "Solid Color",           // 2
    "Short Circuit",         // 3
    "Confetti Red/White",    // 4
    "Rainbow",               // 5
    "Rainbow with Glitter",  // 6
    "Confetti",              // 7
    "Juggle",                // 8
    "Audio Sync",            // 9
    "Solid Flash",           // 10
    "Knight Rider",          // 11
    "Breathing",             // 12
    "Matrix Rain",           // 13
    "Strobe",                // 14
    "Audio VU Meter",        // 15
    "Custom Block Sequence", // 16
    "Plasma",                // 17
    "Fire",                  // 18
    "Twinkle"                // 19
};

// Pattern properties for demo mode
const PatternProperties patternProps[NUM_PATTERNS] = {
    {false, nullptr, nullptr},                    // 0: LEDs Off
    {false, nullptr, nullptr},                    // 1: Random Blocks
    {true, &solidColorIndex, "Solid Color"},      // 2: Solid Color
    {true, &shortColorIndex, "Short Circuit"},    // 3: Short Circuit
    {true, &confettiColor1, "Confetti"},          // 4: Confetti
    {false, nullptr, nullptr},                    // 5: Rainbow
    {false, nullptr, nullptr},                    // 6: Rainbow with Glitter
    {false, nullptr, nullptr},                    // 7: Confetti
    {false, nullptr, nullptr},                    // 8: Juggle
    {false, nullptr, nullptr},                    // 9: Audio Sync
    {true, &flashColorIndex, "Solid Flash"},      // 10: Solid Flash
    {true, &knightColorIndex, "Knight Rider"},    // 11: Knight Rider
    {true, &breathingColorIndex, "Breathing"},    // 12: Breathing
    {true, &matrixColorIndex, "Matrix Rain"},     // 13: Matrix Rain
    {true, &strobeColorIndex, "Strobe"},          // 14: Strobe
    {false, nullptr, nullptr},                    // 15: Audio VU Meter
    {false, nullptr, nullptr},                    // 16: Custom Block Sequence
    {false, nullptr, nullptr},                    // 17: Plasma
    {false, nullptr, nullptr},                    // 18: Fire
    {false, nullptr, nullptr}                     // 19: Twinkle
};

// Thread synchronization
SemaphoreHandle_t ledMutex = nullptr;
volatile bool patternUpdating = false;

// Demo mode state preservation
uint8_t savedPattern = 16;
bool savedDemoState = false;

// Smooth transitions
uint8_t colorFadeSteps = 8;
bool enableSmoothTransitions = true;

// System state
bool systemReady = false;
unsigned long bootTime = 0;

// Serial command globals
String inputString = "";
boolean stringComplete = false;

uint32_t failedCommands = 0;

// Hardware validation function
void validateHardwareConfiguration() {
    Serial.println(F("=== Hardware Configuration Validation ==="));
    
    // Validate LED array pointers
    Serial.print(F("Body LEDs total: "));
    Serial.println(TOTAL_BODY_LEDS);
    
    Serial.print(F("Right panel pointer: "));
    Serial.print((uintptr_t)DJLEDs_Right, HEX);
    Serial.print(F(" (offset: "));
    Serial.print(DJLEDs_Right - bodyLEDsChained);
    Serial.println(F(")"));
    
    Serial.print(F("Middle panel pointer: "));
    Serial.print((uintptr_t)DJLEDs_Middle, HEX);
    Serial.print(F(" (offset: "));
    Serial.print(DJLEDs_Middle - bodyLEDsChained);
    Serial.println(F(")"));
    
    Serial.print(F("Left panel pointer: "));
    Serial.print((uintptr_t)DJLEDs_Left, HEX);
    Serial.print(F(" (offset: "));
    Serial.print(DJLEDs_Left - bodyLEDsChained);
    Serial.println(F(")"));
    
    Serial.print(F("Mouth pointer: "));
    Serial.print((uintptr_t)DJLEDs_Mouth, HEX);
    Serial.print(F(" (offset: "));
    Serial.print(DJLEDs_Mouth - eyesMouthLEDs);
    Serial.println(F(")"));
    
    Serial.print(F("Eyes pointer: "));
    Serial.print((uintptr_t)DJLEDs_Eyes, HEX);
    Serial.print(F(" (offset: "));
    Serial.print(DJLEDs_Eyes - eyesMouthLEDs);
    Serial.println(F(")"));
    
    // Validate array bounds
    bool validConfig = true;
    
    if (DJLEDs_Right != &bodyLEDsChained[0]) {
        Serial.println(F("ERROR: Right panel pointer incorrect"));
        validConfig = false;
    }
    
    if (DJLEDs_Middle != &bodyLEDsChained[NUM_LEDS_PER_PANEL]) {
        Serial.println(F("ERROR: Middle panel pointer incorrect"));
        validConfig = false;
    }
    
    if (DJLEDs_Left != &bodyLEDsChained[NUM_LEDS_PER_PANEL * 2]) {
        Serial.println(F("ERROR: Left panel pointer incorrect"));
        validConfig = false;
    }
    
    if (DJLEDs_Mouth != &eyesMouthLEDs[0]) {
        Serial.println(F("ERROR: Mouth pointer incorrect"));
        validConfig = false;
    }
    
    if (DJLEDs_Eyes != &eyesMouthLEDs[NUM_MOUTH_LEDS]) {
        Serial.println(F("ERROR: Eyes pointer incorrect"));
        validConfig = false;
    }
    
    Serial.print(F("Hardware configuration: "));
    Serial.println(validConfig ? "VALID" : "INVALID");
    
    if (!validConfig) {
        Serial.println(F("CRITICAL: Hardware configuration errors detected!"));
    }
    
    Serial.println(F("========================================"));
}