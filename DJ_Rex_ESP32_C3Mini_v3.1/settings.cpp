#include "settings.h"

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
    
    // Brightness settings
    eyeBrightness = preferences.getUChar("eyeBright", 125);
    bodyBrightness = preferences.getUChar("bodyBright", 100);
    mouthOuterBoost = preferences.getUChar("mouthOuter", 80);
    mouthInnerBoost = preferences.getUChar("mouthInner", 150);
    
    // Validate ranges
    if (currentPattern >= NUM_PATTERNS) currentPattern = 16;
    if (ledBrightness == 0) ledBrightness = 90;
    if (effectSpeed == 0) effectSpeed = 128;
    if (sideBlinkRate == 0) sideBlinkRate = 128;
    if (blockBlinkRate == 0) blockBlinkRate = 128;
    if (audioMode > AUDIO_ALL) audioMode = AUDIO_ALL;
    if (audioSensitivity == 0 || audioSensitivity > 10) audioSensitivity = 5;
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
    audioSensitivity = 5;
    audioAutoGain = true;
    
    // Reset brightness settings
    eyeBrightness = 125;
    bodyBrightness = 100;
    mouthOuterBoost = 80;
    mouthInnerBoost = 150;
    
    Serial.println(F("Factory reset complete"));
}

void saveUserPreset(uint8_t presetNum) {
    if (presetNum < 1 || presetNum > 3) return;
    String presetKey = "preset" + String(presetNum);
    
    preferences.putUChar((presetKey + "_pattern").c_str(), currentPattern);
    preferences.putUChar((presetKey + "_bright").c_str(), ledBrightness);
    preferences.putUChar((presetKey + "_speed").c_str(), effectSpeed);
    preferences.putUChar((presetKey + "_solid").c_str(), solidColorIndex);
    preferences.putUChar((presetKey + "_eye").c_str(), eyeColorIndex);
    preferences.putUChar((presetKey + "_mouth").c_str(), mouthPattern);
    preferences.putUChar((presetKey + "_mouthCol").c_str(), mouthColorIndex);
    preferences.putUChar((presetKey + "_audio").c_str(), audioMode);
    preferences.putUChar((presetKey + "_side1").c_str(), sideColor1);
    preferences.putUChar((presetKey + "_side2").c_str(), sideColor2);
    preferences.putUChar((presetKey + "_side3").c_str(), sideColor3);
    preferences.putUChar((presetKey + "_sideMode").c_str(), sideColorMode);
    preferences.putUChar((presetKey + "_fadeSpd").c_str(), fadeSpeed);
    preferences.putUChar((presetKey + "_conf1").c_str(), confettiColor1);
    preferences.putUChar((presetKey + "_conf2").c_str(), confettiColor2);
    
    // Save eye flicker settings in presets
    preferences.putBool((presetKey + "_eyeFlick").c_str(), eyeFlickerEnabled);
    preferences.putUShort((presetKey + "_eyeFlickMin").c_str(), eyeFlickerMinTime);
    preferences.putUShort((presetKey + "_eyeFlickMax").c_str(), eyeFlickerMaxTime);
    preferences.putUChar((presetKey + "_eyeStaticBr").c_str(), eyeStaticBrightness);

    // NEW: Save advanced settings in presets
    preferences.putUChar((presetKey + "_eye2").c_str(), eyeColorIndex2);
    preferences.putUChar((presetKey + "_eyeMode").c_str(), eyeMode);
    preferences.putUChar((presetKey + "_mouthCol2").c_str(), mouthColorIndex2);
    preferences.putUChar((presetKey + "_mouthSplit").c_str(), mouthSplitMode);
    preferences.putUChar((presetKey + "_waveSpd").c_str(), waveSpeed);
    preferences.putUChar((presetKey + "_pulseSpd").c_str(), pulseSpeed);
    
    preferences.putBytes((presetKey + "_blocks").c_str(), blockColors, 9);
    preferences.putBool((presetKey + "_saved").c_str(), true);
    
    Serial.print(F("User preset "));
    Serial.print(presetNum);
    Serial.println(F(" saved"));
}

bool loadUserPreset(uint8_t presetNum) {
    if (presetNum < 1 || presetNum > 3) return false;
    String presetKey = "preset" + String(presetNum);
    
    if (!preferences.getBool((presetKey + "_saved").c_str(), false)) {
        Serial.print(F("User preset "));
        Serial.print(presetNum);
        Serial.println(F(" is empty"));
        return false;
    }
    
    currentPattern = preferences.getUChar((presetKey + "_pattern").c_str(), currentPattern);
    ledBrightness = preferences.getUChar((presetKey + "_bright").c_str(), ledBrightness);
    effectSpeed = preferences.getUChar((presetKey + "_speed").c_str(), effectSpeed);
    solidColorIndex = preferences.getUChar((presetKey + "_solid").c_str(), solidColorIndex);
    eyeColorIndex = preferences.getUChar((presetKey + "_eye").c_str(), eyeColorIndex);
    mouthPattern = preferences.getUChar((presetKey + "_mouth").c_str(), mouthPattern);
    mouthColorIndex = preferences.getUChar((presetKey + "_mouthCol").c_str(), mouthColorIndex);
    audioMode = preferences.getUChar((presetKey + "_audio").c_str(), audioMode);
    sideColor1 = preferences.getUChar((presetKey + "_side1").c_str(), sideColor1);
    sideColor2 = preferences.getUChar((presetKey + "_side2").c_str(), sideColor2);
    sideColor3 = preferences.getUChar((presetKey + "_side3").c_str(), sideColor3);
    sideColorMode = preferences.getUChar((presetKey + "_sideMode").c_str(), sideColorMode);
    fadeSpeed = preferences.getUChar((presetKey + "_fadeSpd").c_str(), fadeSpeed);
    confettiColor1 = preferences.getUChar((presetKey + "_conf1").c_str(), confettiColor1);
    confettiColor2 = preferences.getUChar((presetKey + "_conf2").c_str(), confettiColor2);
    
    // Load eye flicker settings from presets
    eyeFlickerEnabled = preferences.getBool((presetKey + "_eyeFlick").c_str(), eyeFlickerEnabled);
    eyeFlickerMinTime = preferences.getUShort((presetKey + "_eyeFlickMin").c_str(), eyeFlickerMinTime);
    eyeFlickerMaxTime = preferences.getUShort((presetKey + "_eyeFlickMax").c_str(), eyeFlickerMaxTime);
    eyeStaticBrightness = preferences.getUChar((presetKey + "_eyeStaticBr").c_str(), eyeStaticBrightness);

    // NEW: Load advanced settings from presets
    eyeColorIndex2 = preferences.getUChar((presetKey + "_eye2").c_str(), eyeColorIndex2);
    eyeMode = preferences.getUChar((presetKey + "_eyeMode").c_str(), eyeMode);
    mouthColorIndex2 = preferences.getUChar((presetKey + "_mouthCol2").c_str(), mouthColorIndex2);
    mouthSplitMode = preferences.getUChar((presetKey + "_mouthSplit").c_str(), mouthSplitMode);
    waveSpeed = preferences.getUChar((presetKey + "_waveSpd").c_str(), waveSpeed);
    pulseSpeed = preferences.getUChar((presetKey + "_pulseSpd").c_str(), pulseSpeed);
    
    preferences.getBytes((presetKey + "_blocks").c_str(), blockColors, 9);
    
    FastLED.setBrightness(ledBrightness);
    demoMode = false;
    
    Serial.print(F("User preset "));
    Serial.print(presetNum);
    Serial.println(F(" loaded"));
    
    return true;
}

void deleteUserPreset(uint8_t presetNum) {
    if (presetNum < 1 || presetNum > 3) return;
    String presetKey = "preset" + String(presetNum);
    preferences.putBool((presetKey + "_saved").c_str(), false);
    
    Serial.print(F("User preset "));
    Serial.print(presetNum);
    Serial.println(F(" deleted"));
}