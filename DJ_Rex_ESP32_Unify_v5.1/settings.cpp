#include "settings.h"
#include "preset_manager.h"  // v5.0.1: For unified preset system

void initSettings() {
    preferences.begin("djrex", false);
    loadSettings();
}

void loadSettings() {
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
    eyeColorIndex = preferences.getUChar("eyeColor", 14);
    solidMode = preferences.getUChar("solidMode", 0);
    
    // Eye flicker settings
    eyeFlickerEnabled = preferences.getBool("eyeFlicker", false);
    eyeFlickerMinTime = preferences.getUShort("eyeFlickMin", 200);
    eyeFlickerMaxTime = preferences.getUShort("eyeFlickMax", 1600);
    eyeStaticBrightness = preferences.getUChar("eyeStaticBr", 255);

    // NEW: Load advanced eye settings
    eyeColorIndex2 = preferences.getUChar("eyeColor2", 12);
    eyeMode = preferences.getUChar("eyeMode", 0);
    
    // Demo mode
    demoMode = preferences.getBool("demoMode", false);
    demoTime = preferences.getUShort("demoTime", 10);
    
    // Pattern specific
    flashColorIndex = preferences.getUChar("flashColor", 0);
    flashSpeed = preferences.getUChar("flashSpeed", 5);
    shortColorIndex = preferences.getUChar("shortColor", 3);
    knightColorIndex = preferences.getUChar("knightColor", 0);
    breathingColorIndex = preferences.getUChar("breathColor", 2);
    matrixColorIndex = preferences.getUChar("matrixColor", 1);
    strobeColorIndex = preferences.getUChar("strobeColor", 3);
    
    // Block colors
    preferences.getBytes("blockColors", blockColors, 9);
    
    // Side LED settings
    sideColor1 = preferences.getUChar("sideColor1", 0);
    sideColor2 = preferences.getUChar("sideColor2", 2);
    sideColor3 = preferences.getUChar("sideColor3", 3);
    sideColorMode = preferences.getUChar("sideMode", 0);
    sideBlinkRate = preferences.getUChar("sideRate", 128);
    blockBlinkRate = preferences.getUChar("blockRate", 128);
    
    // Mouth settings
    mouthPattern = preferences.getUChar("mouthPattern", 1);
    mouthColorIndex = preferences.getUChar("mouthColor", 0);
    mouthBrightness = preferences.getUChar("mouthBright", 90);
    mouthEnabled = preferences.getBool("mouthEnabled", true);
    talkSpeed = preferences.getUChar("talkSpeed", 5);
    smileWidth = preferences.getUChar("smileWidth", 6);

    // NEW: Load advanced mouth settings
    mouthColorIndex2 = preferences.getUChar("mouthColor2", 3);
    mouthSplitMode = preferences.getUChar("mouthSplit", 0);
    waveSpeed = preferences.getUChar("waveSpeed", 5);
    pulseSpeed = preferences.getUChar("pulseSpeed", 5);
    
    // Audio settings
    audioThreshold = preferences.getUShort("audioThresh", 100);
    audioMode = preferences.getUChar("audioMode", AUDIO_ALL);
    audioSensitivity = preferences.getUChar("audioSens", 5);
    audioAutoGain = preferences.getBool("autoGain", true);
    audioInputMode = preferences.getUChar("audioInput", INPUT_MIC);  // v5.1
    
    // Brightness settings
    eyeBrightness = preferences.getUChar("eyeBright", 125);
    bodyBrightness = preferences.getUChar("bodyBright", 100);
    mouthOuterBoost = preferences.getUChar("mouthOuter", 80);
    mouthInnerBoost = preferences.getUChar("mouthInner", 150);

    // v5.0: Startup sequence setting
    startupSequenceEnabled = preferences.getBool("startupSeq", STARTUP_SEQUENCE_ENABLED);

    // Validate ranges
    if (currentPattern >= NUM_PATTERNS) currentPattern = 16;
    if (ledBrightness == 0) ledBrightness = 90;
    if (effectSpeed == 0) effectSpeed = 128;
    if (sideBlinkRate == 0) sideBlinkRate = 128;
    if (blockBlinkRate == 0) blockBlinkRate = 128;
    if (audioMode > AUDIO_ALL) audioMode = AUDIO_ALL;
    if (audioSensitivity == 0 || audioSensitivity > 10) audioSensitivity = 5;
    if (audioInputMode > INPUT_LINE_IN) audioInputMode = INPUT_MIC;  // v5.1
    if (mouthPattern >= NUM_MOUTH_PATTERNS) mouthPattern = 1;
    if (eyeMode >= 3) eyeMode = 0;
    if (mouthSplitMode >= 5) mouthSplitMode = 0;
    
    // Validate eye flicker settings
    if (eyeFlickerMinTime < 50) eyeFlickerMinTime = 200;
    if (eyeFlickerMaxTime < eyeFlickerMinTime) eyeFlickerMaxTime = eyeFlickerMinTime + 400;
    if (eyeFlickerMaxTime > 5000) eyeFlickerMaxTime = 1600;
    
    Serial.println(F("Settings loaded"));
}

void saveSettings() {
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
    preferences.putUChar("solidMode", solidMode);
    
    // Save eye flicker settings
    preferences.putBool("eyeFlicker", eyeFlickerEnabled);
    preferences.putUShort("eyeFlickMin", eyeFlickerMinTime);
    preferences.putUShort("eyeFlickMax", eyeFlickerMaxTime);
    preferences.putUChar("eyeStaticBr", eyeStaticBrightness);

    // NEW: Save advanced eye settings
    preferences.putUChar("eyeColor2", eyeColorIndex2);
    preferences.putUChar("eyeMode", eyeMode);
    
    preferences.putBool("demoMode", demoMode);
    preferences.putUShort("demoTime", demoTime);
    
    preferences.putUChar("flashColor", flashColorIndex);
    preferences.putUChar("flashSpeed", flashSpeed);
    preferences.putUChar("shortColor", shortColorIndex);
    preferences.putUChar("knightColor", knightColorIndex);
    preferences.putUChar("breathColor", breathingColorIndex);
    preferences.putUChar("matrixColor", matrixColorIndex);
    preferences.putUChar("strobeColor", strobeColorIndex);
    
    preferences.putBytes("blockColors", blockColors, 9);
    
    preferences.putUChar("sideColor1", sideColor1);
    preferences.putUChar("sideColor2", sideColor2);
    preferences.putUChar("sideColor3", sideColor3);
    preferences.putUChar("sideMode", sideColorMode);
    preferences.putUChar("sideRate", sideBlinkRate);
    preferences.putUChar("blockRate", blockBlinkRate);
    
    preferences.putUChar("mouthPattern", mouthPattern);
    preferences.putUChar("mouthColor", mouthColorIndex);
    preferences.putUChar("mouthBright", mouthBrightness);
    preferences.putBool("mouthEnabled", mouthEnabled);
    preferences.putUChar("talkSpeed", talkSpeed);
    preferences.putUChar("smileWidth", smileWidth);

    // NEW: Save advanced mouth settings
    preferences.putUChar("mouthColor2", mouthColorIndex2);
    preferences.putUChar("mouthSplit", mouthSplitMode);
    preferences.putUChar("waveSpeed", waveSpeed);
    preferences.putUChar("pulseSpeed", pulseSpeed);
    
    preferences.putUShort("audioThresh", audioThreshold);
    preferences.putUChar("audioMode", audioMode);
    preferences.putUChar("audioSens", audioSensitivity);
    preferences.putBool("autoGain", audioAutoGain);
    preferences.putUChar("audioInput", audioInputMode);  // v5.1
    
    preferences.putUChar("eyeBright", eyeBrightness);
    preferences.putUChar("bodyBright", bodyBrightness);
    preferences.putUChar("mouthOuter", mouthOuterBoost);
    preferences.putUChar("mouthInner", mouthInnerBoost);
    
    Serial.println(F("Settings saved"));
}

void resetToDefaults() {
    preferences.clear();
    
    currentPattern = 16;
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
    eyeColorIndex = 14;
    solidMode = 0;
    demoMode = false;
    demoTime = 10;
    flashColorIndex = 0;
    flashSpeed = 5;
    shortColorIndex = 3;
    knightColorIndex = 0;
    breathingColorIndex = 2;
    matrixColorIndex = 1;
    strobeColorIndex = 3;
    
    // Reset eye flicker settings
    eyeFlickerEnabled = false;
    eyeFlickerMinTime = 200;
    eyeFlickerMaxTime = 1600;
    eyeStaticBrightness = 255;

    // NEW: Reset advanced eye settings
    eyeColorIndex2 = 12;
    eyeMode = 0;
    
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
    mouthColorIndex = 0;
    mouthBrightness = 90;
    mouthEnabled = true;
    talkSpeed = 5;
    smileWidth = 6;

    // NEW: Reset advanced mouth settings
    mouthColorIndex2 = 3;
    mouthSplitMode = 0;
    waveSpeed = 5;
    pulseSpeed = 5;
    
    // Reset audio settings
    audioMode = AUDIO_ALL;
    audioSensitivity = MIC_DEFAULT_SENSITIVITY;
    audioAutoGain = true;
    audioInputMode = INPUT_MIC;  // v5.1
    
    // Reset brightness settings
    eyeBrightness = 125;
    bodyBrightness = 100;
    mouthOuterBoost = 80;
    mouthInnerBoost = 150;
    
    Serial.println(F("Factory reset complete"));
}

// v5.0.1: Legacy preset functions now map to v5 PresetManager
// This provides backward compatibility while using the new extended preset system
void saveUserPreset(uint8_t presetNum) {
    if (presetNum < 1 || presetNum > 3) return;

    // Map legacy slots 1-3 directly to v5 PresetManager slots 1-3
    // Generate legacy-style name for compatibility
    char legacyName[PRESET_NAME_LENGTH];
    snprintf(legacyName, PRESET_NAME_LENGTH, "User%d", presetNum);

    if (presetManager.savePreset(presetNum, legacyName)) {
        Serial.print(F("User preset "));
        Serial.print(presetNum);
        Serial.println(F(" saved"));
    } else {
        Serial.println(F("Failed to save user preset"));
    }
}

// v5.0.1: Legacy load function now uses v5 PresetManager
bool loadUserPreset(uint8_t presetNum) {
    if (presetNum < 1 || presetNum > 3) return false;

    // Map legacy slots 1-3 to v5 PresetManager slots 1-3
    if (presetManager.loadPreset(presetNum)) {
        Serial.print(F("User preset "));
        Serial.print(presetNum);
        Serial.println(F(" loaded"));
        return true;
    } else {
        Serial.print(F("User preset "));
        Serial.print(presetNum);
        Serial.println(F(" is empty"));
        return false;
    }
}

// v5.0.1: Legacy delete function now uses v5 PresetManager
void deleteUserPreset(uint8_t presetNum) {
    if (presetNum < 1 || presetNum > 3) return;

    // Map legacy slots 1-3 to v5 PresetManager slots 1-3
    if (presetManager.deletePreset(presetNum)) {
        Serial.print(F("User preset "));
        Serial.print(presetNum);
        Serial.println(F(" deleted"));
    } else {
        Serial.println(F("Failed to delete user preset"));
    }
}