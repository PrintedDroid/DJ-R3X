// preset_manager.cpp - v5.0 Extended Preset Manager (10 slots)
#include "preset_manager.h"
#include "event_logger.h"
#include <Preferences.h>

PresetManager presetManager;

void PresetManager::begin() {
    // Initialize all presets as invalid
    for (uint8_t i = 0; i < MAX_PRESETS; i++) {
        presets[i].valid = false;
        snprintf(presets[i].name, PRESET_NAME_LENGTH, "Preset %d", i + 1);
    }

    loadFromFlash();
    Serial.println(F("Preset Manager initialized (10 slots)"));
}

void PresetManager::loadFromFlash() {
    Preferences prefs;
    prefs.begin("presets", true);  // Read-only

    for (uint8_t i = 0; i < MAX_PRESETS; i++) {
        char key[12];
        snprintf(key, sizeof(key), "preset%d", i);

        if (prefs.isKey(key)) {
            size_t len = prefs.getBytes(key, &presets[i], sizeof(ExtendedPreset));
            if (len != sizeof(ExtendedPreset)) {
                presets[i].valid = false;
            }
        }
    }

    prefs.end();
}

void PresetManager::saveToFlash(uint8_t slot) {
    if (slot >= MAX_PRESETS) return;

    Preferences prefs;
    prefs.begin("presets", false);

    char key[12];
    snprintf(key, sizeof(key), "preset%d", slot);
    prefs.putBytes(key, &presets[slot], sizeof(ExtendedPreset));

    prefs.end();
}

bool PresetManager::savePreset(uint8_t slot, const char* name) {
    if (slot >= MAX_PRESETS) {
        Serial.print(F("Invalid preset slot: "));
        Serial.println(slot);
        return false;
    }

    captureCurrentState(presets[slot]);
    presets[slot].valid = true;

    if (name != nullptr && strlen(name) > 0) {
        strncpy(presets[slot].name, name, PRESET_NAME_LENGTH - 1);
        presets[slot].name[PRESET_NAME_LENGTH - 1] = '\0';
    }

    saveToFlash(slot);

    Serial.print(F("Preset saved to slot "));
    Serial.print(slot + 1);
    Serial.print(F(": "));
    Serial.println(presets[slot].name);

    eventLogger.log(EVENT_PRESET_SAVE, slot);
    return true;
}

bool PresetManager::loadPreset(uint8_t slot) {
    if (slot >= MAX_PRESETS) {
        Serial.print(F("Invalid preset slot: "));
        Serial.println(slot);
        return false;
    }

    if (!presets[slot].valid) {
        Serial.print(F("Preset slot "));
        Serial.print(slot + 1);
        Serial.println(F(" is empty"));
        return false;
    }

    applyPreset(presets[slot]);

    Serial.print(F("Loaded preset "));
    Serial.print(slot + 1);
    Serial.print(F(": "));
    Serial.println(presets[slot].name);

    eventLogger.log(EVENT_PRESET_LOAD, slot);
    return true;
}

bool PresetManager::deletePreset(uint8_t slot) {
    if (slot >= MAX_PRESETS) return false;

    presets[slot].valid = false;
    snprintf(presets[slot].name, PRESET_NAME_LENGTH, "Preset %d", slot + 1);

    Preferences prefs;
    prefs.begin("presets", false);
    char key[12];
    snprintf(key, sizeof(key), "preset%d", slot);
    prefs.remove(key);
    prefs.end();

    Serial.print(F("Deleted preset "));
    Serial.println(slot + 1);
    return true;
}

void PresetManager::listPresets() {
    Serial.println(F("=== Presets (10 slots) ==="));
    for (uint8_t i = 0; i < MAX_PRESETS; i++) {
        Serial.print(F("  "));
        Serial.print(i + 1);
        Serial.print(F(": "));
        if (presets[i].valid) {
            Serial.print(presets[i].name);
            Serial.print(F(" [Pattern "));
            Serial.print(presets[i].pattern);
            Serial.println(F("]"));
        } else {
            Serial.println(F("(empty)"));
        }
    }
    Serial.println(F("========================"));
}

bool PresetManager::isValidSlot(uint8_t slot) {
    return slot < MAX_PRESETS && presets[slot].valid;
}

const char* PresetManager::getPresetName(uint8_t slot) {
    if (slot >= MAX_PRESETS) return "Invalid";
    return presets[slot].name;
}

void PresetManager::captureCurrentState(ExtendedPreset& preset) {
    preset.pattern = currentPattern;
    preset.brightness = ledBrightness;
    preset.speed = effectSpeed;
    preset.solidColor = solidColorIndex;
    preset.eyeColor = eyeColorIndex;
    preset.eyeColor2 = eyeColorIndex2;
    preset.eyeMode = eyeMode;
    preset.mouthPattern = mouthPattern;
    preset.mouthColor = mouthColorIndex;
    preset.mouthColor2 = mouthColorIndex2;
    preset.mouthSplitMode = mouthSplitMode;
    preset.audioMode = audioMode;

    for (uint8_t i = 0; i < 9; i++) {
        preset.blockColors[i] = blockColors[i];
    }

    preset.sideColors[0] = sideColor1;
    preset.sideColors[1] = sideColor2;
    preset.sideColors[2] = sideColor3;
    preset.sideMode = sideColorMode;
}

void PresetManager::applyPreset(const ExtendedPreset& preset) {
    currentPattern = preset.pattern;
    ledBrightness = preset.brightness;
    effectSpeed = preset.speed;
    solidColorIndex = preset.solidColor;
    eyeColorIndex = preset.eyeColor;
    eyeColorIndex2 = preset.eyeColor2;
    eyeMode = preset.eyeMode;
    mouthPattern = preset.mouthPattern;
    mouthColorIndex = preset.mouthColor;
    mouthColorIndex2 = preset.mouthColor2;
    mouthSplitMode = preset.mouthSplitMode;
    audioMode = preset.audioMode;

    for (uint8_t i = 0; i < 9; i++) {
        blockColors[i] = preset.blockColors[i];
    }

    sideColor1 = preset.sideColors[0];
    sideColor2 = preset.sideColors[1];
    sideColor3 = preset.sideColors[2];
    sideColorMode = preset.sideMode;

    FastLED.setBrightness(ledBrightness);
}
