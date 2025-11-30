#include "settings.h"
#include "status_led.h"

// Settings validation helper
bool validateSettings() {
    bool valid = true;
    
    if (currentPattern >= NUM_PATTERNS) {
        currentPattern = 16;
        valid = false;
    }
    if (ledBrightness == 0) {
        ledBrightness = 90;
        valid = false;
    }
    if (effectSpeed == 0) {
        effectSpeed = 128;
        valid = false;
    }
    if (sideBlinkRate == 0) {
        sideBlinkRate = 128;
        valid = false;
    }
    if (blockBlinkRate == 0) {
        blockBlinkRate = 128;
        valid = false;
    }
    if (audioMode >= 6) { 
        audioMode = AUDIO_ALL;
        valid = false;
    }
    if (audioSensitivity == 0 || audioSensitivity > 10) {
        audioSensitivity = 5;
        valid = false;
    }
    if (mouthPattern >= NUM_MOUTH_PATTERNS) {
        mouthPattern = 1;
        valid = false;
    }
    if (mouthSplitMode > 4) {
        mouthSplitMode = MOUTH_SPLIT_OFF;
        valid = false;
    }
    if (eyeMode > 6) {
        eyeMode = EYE_MODE_SINGLE;
        valid = false;
    }
    if (eyeColorIndex >= NUM_STANDARD_COLORS) {
        eyeColorIndex = 3;
        valid = false;
    }
    if (eyeColorIndex2 >= NUM_STANDARD_COLORS) {
        eyeColorIndex2 = 2;
        valid = false;
    }
    if (audioThreshold < 10) { // Using a safe minimum instead of AUDIO_MIN_THRESHOLD from a potentially removed file
        audioThreshold = 50;
        valid = false;
    }
    if (audioThreshold > 1000) { // Using a safe maximum instead of AUDIO_MAX_THRESHOLD
        audioThreshold = 500;
        valid = false;
    }
    
    // Validate all color indices
    for (int i = 0; i < 9; i++) {
        if (blockColors[i] >= NUM_STANDARD_COLORS && blockColors[i] != 10) {
            blockColors[i] = i % NUM_STANDARD_COLORS;
            valid = false;
        }
    }
    if (eyeFlickerMinTime < 50) {
    eyeFlickerMinTime = 200;
    valid = false;
}
if (eyeFlickerMaxTime < eyeFlickerMinTime) {
    eyeFlickerMaxTime = eyeFlickerMinTime + 400;
    valid = false;
}
if (eyeFlickerMaxTime > 5000) {
    eyeFlickerMaxTime = 1600;
    valid = false;
}

    return valid;
}

void initSettings() {
    preferences.begin("djrex", false);
    
    // Check if settings exist
    if (!preferences.getBool("initialized", false)) {
        Serial.println(F("First boot - initializing settings"));
        resetToDefaults();
        saveSettings();
        preferences.putBool("initialized", true);
    } else {
        loadSettings();
    }
}

void loadSettings() {
    Serial.println(F("Loading settings..."));
    
    // Basic settings
    currentPattern = preferences.getUChar("pattern", 16);
    ledBrightness = preferences.getUChar("brightness", 90);
    effectSpeed = preferences.getUChar("speed", 128);
    sideMinTime = preferences.getUShort("sideMin", 500);
    sideMaxTime = preferences.getUShort("sideMax", 2500);
    blockMinTime = preferences.getUShort("blockMin", 200);
    blockMaxTime = preferences.getUShort("blockMax", 1500);
    fadeSpeed = preferences.getUChar("fadeSpeed", 8);
    solidColorIndex = preferences.getUChar("solidColor", 0);
    confettiColor1 = preferences.getUChar("confetti1", 0);
    confettiColor2 = preferences.getUChar("confetti2", 3);
    eyeColorIndex = preferences.getUChar("eyeColor", 3);
    eyeColorIndex2 = preferences.getUChar("eyeColor2", 2);
    eyeMode = preferences.getUChar("eyeMode", EYE_MODE_SINGLE);
    solidMode = preferences.getUChar("solidMode", 0);
    eyeFlickerEnabled = preferences.getBool("eyeFlicker", true);
    eyeFlickerMinTime = preferences.getUShort("eyeFlickMin", 200);
    eyeFlickerMaxTime = preferences.getUShort("eyeFlickMax", 1600);
    eyeStaticBrightness = preferences.getUChar("eyeStaticBr", 255);

    
    // Demo mode
    demoMode = preferences.getBool("demoMode", false);
    demoTime = preferences.getUShort("demoTime", 10);
    
    // Pattern specific
    flashColorIndex = preferences.getUChar("flashColor", 0);
    flashSpeed = preferences.getUChar("flashSpeed", 5);
    shortColorIndex = preferences.getUChar("shortColor", 3);
    knightColorIndex = preferences.getUChar("knightColor", 0);
    breathingColorIndex = preferences.getUChar("breathColor", 2);
    matrixColorIndex = preferences.getUChar("matrixColor", 11);
    strobeColorIndex = preferences.getUChar("strobeColor", 3);
    
    // Block colors with error checking
    size_t len = preferences.getBytes("blockColors", blockColors, 9);
    if (len != 9) {
        Serial.println(F("Warning: Block colors corrupted, resetting"));
        for (int i = 0; i < 9; i++) {
            blockColors[i] = i % 10;
        }
    }
    
    // Side LED settings
    sideColor1 = preferences.getUChar("sideColor1", 0);
    sideColor2 = preferences.getUChar("sideColor2", 2);
    sideColor3 = preferences.getUChar("sideColor3", 3);
    sideColorMode = preferences.getUChar("sideMode", 0);
    sideBlinkRate = preferences.getUChar("sideRate", 128);
    blockBlinkRate = preferences.getUChar("blockRate", 128);
    
    // Mouth settings
    mouthPattern = preferences.getUChar("mouthPattern", 1);
    mouthColorIndex = preferences.getUChar("mouthColor", 2);
    mouthColorIndex2 = preferences.getUChar("mouthColor2", 0);
    mouthSplitMode = preferences.getUChar("mouthSplit", MOUTH_SPLIT_OFF);
    mouthBrightness = preferences.getUChar("mouthBright", 90);
    mouthEnabled = preferences.getBool("mouthEnabled", true);
    talkSpeed = preferences.getUChar("talkSpeed", 5);
    smileWidth = preferences.getUChar("smileWidth", 6);
    waveSpeed = preferences.getUChar("waveSpeed", 5);
    pulseSpeed = preferences.getUChar("pulseSpeed", 5);
    
    // Audio settings
    audioThreshold = preferences.getUShort("audioThresh", 100);
    audioMode = preferences.getUChar("audioMode", AUDIO_ALL);
    audioSensitivity = preferences.getUChar("audioSens", 5);
    audioAutoGain = preferences.getBool("autoGain", true);
    
    // Brightness settings
    eyeBrightness = preferences.getUChar("eyeBright", 125);
    bodyBrightness = preferences.getUChar("bodyBright", 100);
    mouthOuterBoost = preferences.getUChar("mouthOuter", 100);
    mouthInnerBoost = preferences.getUChar("mouthInner", 150);
    
    // Validate all settings
    if (!validateSettings()) {
        Serial.println(F("Warning: Some settings were invalid and corrected"));
        setStatusLED(STATUS_ERROR);
    }
    
    Serial.println(F("Settings loaded successfully"));
}

void saveSettings() {
    Serial.println(F("Saving settings..."));
    
    // Validate before saving
    validateSettings();
    
    preferences.putUChar("pattern", currentPattern);
    preferences.putUChar("brightness", ledBrightness);
    preferences.putUChar("speed", effectSpeed);
    preferences.putUShort("sideMin", sideMinTime);
    preferences.putUShort("sideMax", sideMaxTime);
    preferences.putUShort("blockMin", blockMinTime);
    preferences.putUShort("blockMax", blockMaxTime);
    preferences.putUChar("fadeSpeed", fadeSpeed);
    preferences.putUChar("solidColor", solidColorIndex);
    preferences.putUChar("confetti1", confettiColor1);
    preferences.putUChar("confetti2", confettiColor2);
    preferences.putUChar("eyeColor", eyeColorIndex);
    preferences.putUChar("eyeColor2", eyeColorIndex2);
    preferences.putUChar("eyeMode", eyeMode);
    preferences.putUChar("solidMode", solidMode);
    preferences.putBool("eyeFlicker", eyeFlickerEnabled);
    preferences.putUShort("eyeFlickMin", eyeFlickerMinTime);
    preferences.putUShort("eyeFlickMax", eyeFlickerMaxTime);
    preferences.putUChar("eyeStaticBr", eyeStaticBrightness);
    
    preferences.putBool("demoMode", demoMode);
    preferences.putUShort("demoTime", demoTime);
    
    preferences.putUChar("flashColor", flashColorIndex);
    preferences.putUChar("flashSpeed", flashSpeed);
    preferences.putUChar("shortColor", shortColorIndex);
    preferences.putUChar("knightColor", knightColorIndex);
    preferences.putUChar("breathColor", breathingColorIndex);
    preferences.putUChar("matrixColor", matrixColorIndex);
    preferences.putUChar("strobeColor", strobeColorIndex);
    
    size_t written = preferences.putBytes("blockColors", blockColors, 9);
    if (written != 9) {
        Serial.println(F("Error: Failed to save block colors"));
        setStatusLED(STATUS_ERROR);
    }
    
    preferences.putUChar("sideColor1", sideColor1);
    preferences.putUChar("sideColor2", sideColor2);
    preferences.putUChar("sideColor3", sideColor3);
    preferences.putUChar("sideMode", sideColorMode);
    preferences.putUChar("sideRate", sideBlinkRate);
    preferences.putUChar("blockRate", blockBlinkRate);
    
    preferences.putUChar("mouthPattern", mouthPattern);
    preferences.putUChar("mouthColor", mouthColorIndex);
    preferences.putUChar("mouthColor2", mouthColorIndex2);
    preferences.putUChar("mouthSplit", mouthSplitMode);
    preferences.putUChar("mouthBright", mouthBrightness);
    preferences.putBool("mouthEnabled", mouthEnabled);
    preferences.putUChar("talkSpeed", talkSpeed);
    preferences.putUChar("smileWidth", smileWidth);
    preferences.putUChar("waveSpeed", waveSpeed);
    preferences.putUChar("pulseSpeed", pulseSpeed);
    
    preferences.putUShort("audioThresh", audioThreshold);
    preferences.putUChar("audioMode", audioMode);
    preferences.putUChar("audioSens", audioSensitivity);
    preferences.putBool("autoGain", audioAutoGain);
    
    preferences.putUChar("eyeBright", eyeBrightness);
    preferences.putUChar("bodyBright", bodyBrightness);
    preferences.putUChar("mouthOuter", mouthOuterBoost);
    preferences.putUChar("mouthInner", mouthInnerBoost);
    
    Serial.println(F("Settings saved successfully"));
}

void resetToDefaults() {
    Serial.println(F("Resetting to factory defaults..."));
    
    // Clear all preferences
    preferences.clear();
    
    // Reset all to defaults
    currentPattern = 16;  // Default to Custom Block Sequence
    ledBrightness = 90;
    effectSpeed = 128;
    sideMinTime = 500;
    sideMaxTime = 2500;
    blockMinTime = 200;
    blockMaxTime = 1500;
    fadeSpeed = 8;
    solidColorIndex = 0;
    audioThreshold = 100;
    confettiColor1 = 0;
    confettiColor2 = 3;
    eyeColorIndex = 3;  // Default to White
    eyeColorIndex2 = 2; // Default to Blue
    eyeMode = EYE_MODE_SINGLE;
    solidMode = 0;
    eyeFlickerEnabled = true;
    eyeFlickerMinTime = 200;
    eyeFlickerMaxTime = 1600;
    eyeStaticBrightness = 255;
    demoMode = false;
    demoTime = 10;
    flashColorIndex = 0;
    flashSpeed = 5;
    shortColorIndex = 3;
    knightColorIndex = 0;
    breathingColorIndex = 2;
    matrixColorIndex = 11; // Matrix Green
    strobeColorIndex = 3;
    
    // Reset block colors
    for (int i = 0; i < 9; i++) {
        blockColors[i] = i % 10;
    }
    
    // Reset side LED settings
    sideColor1 = 0;
    sideColor2 = 2;
    sideColor3 = 3;
    sideColorMode = 0;
    sideBlinkRate = 128;
    blockBlinkRate = 128;
    
    // Reset mouth settings
    mouthPattern = 1;
    mouthColorIndex = 2;  // Blue
    mouthColorIndex2 = 0; // Red as secondary
    mouthSplitMode = MOUTH_SPLIT_OFF;
    mouthBrightness = 90;
    mouthEnabled = true;
    talkSpeed = 5;
    smileWidth = 6;
    waveSpeed = 5;
    pulseSpeed = 5;
    
    // Reset audio settings
    audioMode = AUDIO_ALL;
    audioSensitivity = 5;
    audioAutoGain = true;
    
    // Reset brightness settings
    eyeBrightness = 125;
    bodyBrightness = 100;
    mouthOuterBoost = 100;
    mouthInnerBoost = 150;
    
    Serial.println(F("Factory reset complete"));
}