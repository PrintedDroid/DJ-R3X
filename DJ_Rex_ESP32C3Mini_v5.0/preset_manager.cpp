#include "preset_manager.h"
#include "globals.h"
#include "settings.h"
#include "event_logger.h"

PresetManager presetManager;

// Static constants
const char PresetManager::STORAGE_PREFIX[] = "preset";

PresetManager::PresetManager() {
    initialized = false;
    totalOperations = 0;
    successfulOperations = 0;
    failedOperations = 0;
    lastOperation = 0;
    
    // Clear all presets
    for (int i = 0; i < MAX_PRESETS; i++) {
        presets[i].used = false;
        presets[i].name[0] = '\0';
        presets[i].timestamp = 0;
        presets[i].checksum = 0;
    }
}

PresetManager::~PresetManager() {
    // Destructor - cleanup if needed
    // prefs is stack allocated, so no manual cleanup needed
}

void PresetManager::begin() {
    Serial.println(F("PresetManager: Initializing..."));
    
    totalOperations = 0;
    successfulOperations = 0;
    failedOperations = 0;
    
    // Load all presets from storage
    for (int i = 0; i < MAX_PRESETS; i++) {
        if (loadPresetFromStorage(i)) {
            if (presets[i].used) {
                Serial.print(F("PresetManager: Loaded slot "));
                Serial.print(i);
                Serial.print(F(" - "));
                Serial.println(presets[i].name);
            }
        } else {
            Serial.print(F("PresetManager: Failed to load slot "));
            Serial.println(i);
        }
    }
    
    initialized = true;
    
    uint8_t usedSlots = getUsedSlotCount();
    Serial.print(F("PresetManager: Initialized with "));
    Serial.print(usedSlots);
    Serial.print(F("/"));
    Serial.print(MAX_PRESETS);
    Serial.println(F(" slots used"));
}

PresetResult PresetManager::savePreset(uint8_t slot, const char* name) {
    totalOperations++;
    lastOperation = millis();
    
    // Validate slot
    if (slot >= MAX_PRESETS) {
        failedOperations++;
        logPresetOperation("save", slot, PRESET_ERROR_INVALID_SLOT);
        return PRESET_ERROR_INVALID_SLOT;
    }
    
    // Validate name
    if (!name || strlen(name) == 0) {
        failedOperations++;
        logPresetOperation("save", slot, PRESET_ERROR_NAME_TOO_LONG);
        return PRESET_ERROR_NAME_TOO_LONG;
    }
    
    if (strlen(name) >= PRESET_NAME_LENGTH) {
        failedOperations++;
        logPresetOperation("save", slot, PRESET_ERROR_NAME_TOO_LONG);
        return PRESET_ERROR_NAME_TOO_LONG;
    }
    
    // Initialize preset with defaults
    initializePresetDefaults(&presets[slot]);
    
    // Copy current settings to preset
    copyCurrentToPreset(&presets[slot]);
    
    // Set metadata
    strncpy(presets[slot].name, name, PRESET_NAME_LENGTH - 1);
    presets[slot].name[PRESET_NAME_LENGTH - 1] = '\0';
    sanitizePresetName(presets[slot].name);
    
    presets[slot].timestamp = millis();
    presets[slot].used = true;
    presets[slot].version = PRESET_VERSION;
    presets[slot].checksum = calculateChecksum(&presets[slot]);
    
    // Validate preset before saving
    if (!validatePreset(&presets[slot])) {
        failedOperations++;
        logPresetOperation("save", slot, PRESET_ERROR_CORRUPTED);
        return PRESET_ERROR_CORRUPTED;
    }
    
    // Save to storage
    if (!savePresetToStorage(slot)) {
        failedOperations++;
        logPresetOperation("save", slot, PRESET_ERROR_STORAGE_FAILED);
        return PRESET_ERROR_STORAGE_FAILED;
    }
    
    successfulOperations++;
    logPresetOperation("save", slot, PRESET_SUCCESS);
    eventLogger.log(EVENT_PRESET_SAVE, String("Saved: ") + name + " (slot " + String(slot) + ")");
    
    return PRESET_SUCCESS;
}

PresetResult PresetManager::loadPreset(uint8_t slot) {
    totalOperations++;
    lastOperation = millis();
    
    // Validate slot
    if (slot >= MAX_PRESETS) {
        failedOperations++;
        logPresetOperation("load", slot, PRESET_ERROR_INVALID_SLOT);
        return PRESET_ERROR_INVALID_SLOT;
    }
    
    // Check if slot is used
    if (!presets[slot].used) {
        failedOperations++;
        logPresetOperation("load", slot, PRESET_ERROR_NOT_FOUND);
        return PRESET_ERROR_NOT_FOUND;
    }
    
    // Verify checksum
    uint16_t calcChecksum = calculateChecksum(&presets[slot]);
    if (calcChecksum != presets[slot].checksum) {
        Serial.print(F("PresetManager: Checksum mismatch in slot "));
        Serial.print(slot);
        Serial.print(F(" - calculated: "));
        Serial.print(calcChecksum);
        Serial.print(F(", stored: "));
        Serial.println(presets[slot].checksum);
        
        // Attempt to reload from storage
        if (loadPresetFromStorage(slot)) {
            calcChecksum = calculateChecksum(&presets[slot]);
            if (calcChecksum == presets[slot].checksum) {
                Serial.println(F("PresetManager: Checksum recovered from storage"));
            } else {
                failedOperations++;
                logPresetOperation("load", slot, PRESET_ERROR_CHECKSUM_FAILED);
                eventLogger.log(EVENT_ERROR, "Preset checksum failed - slot " + String(slot));
                return PRESET_ERROR_CHECKSUM_FAILED;
            }
        } else {
            failedOperations++;
            logPresetOperation("load", slot, PRESET_ERROR_CORRUPTED);
            return PRESET_ERROR_CORRUPTED;
        }
    }
    
    // Validate preset structure
    if (!validatePreset(&presets[slot])) {
        failedOperations++;
        logPresetOperation("load", slot, PRESET_ERROR_CORRUPTED);
        return PRESET_ERROR_CORRUPTED;
    }
    
    // Apply preset to current settings
    if (!applyPresetToCurrent(&presets[slot])) {
        failedOperations++;
        logPresetOperation("load", slot, PRESET_ERROR_CORRUPTED);
        return PRESET_ERROR_CORRUPTED;
    }
    
    // Save current settings to ensure persistence
    saveSettings();
    
    successfulOperations++;
    logPresetOperation("load", slot, PRESET_SUCCESS);
    eventLogger.log(EVENT_PRESET_LOAD, String("Loaded: ") + presets[slot].name + " (slot " + String(slot) + ")");
    
    return PRESET_SUCCESS;
}

PresetResult PresetManager::deletePreset(uint8_t slot) {
    totalOperations++;
    lastOperation = millis();
    
    if (slot >= MAX_PRESETS) {
        failedOperations++;
        logPresetOperation("delete", slot, PRESET_ERROR_INVALID_SLOT);
        return PRESET_ERROR_INVALID_SLOT;
    }
    
    String presetName = String(presets[slot].name);
    
    // Clear preset in memory
    presets[slot].used = false;
    presets[slot].name[0] = '\0';
    presets[slot].timestamp = 0;
    presets[slot].checksum = 0;
    
    // Clear from storage
    String storageKey = String(STORAGE_PREFIX) + String(slot);
    prefs.begin(storageKey.c_str(), false);
    prefs.clear();
    prefs.end();
    
    successfulOperations++;
    logPresetOperation("delete", slot, PRESET_SUCCESS);
    eventLogger.log(EVENT_PRESET_SAVE, String("Deleted: ") + presetName + " (slot " + String(slot) + ")");
    
    return PRESET_SUCCESS;
}

PresetResult PresetManager::copyPreset(uint8_t fromSlot, uint8_t toSlot) {
    totalOperations++;
    lastOperation = millis();
    
    if (fromSlot >= MAX_PRESETS || toSlot >= MAX_PRESETS) {
        failedOperations++;
        return PRESET_ERROR_INVALID_SLOT;
    }
    
    if (!presets[fromSlot].used) {
        failedOperations++;
        return PRESET_ERROR_NOT_FOUND;
    }
    
    // Copy preset
    memcpy(&presets[toSlot], &presets[fromSlot], sizeof(ExtendedPreset));
    
    // Update metadata
    String newName = String(presets[fromSlot].name) + " Copy";
    if (newName.length() >= PRESET_NAME_LENGTH) {
        newName = newName.substring(0, PRESET_NAME_LENGTH - 1);
    }
    strcpy(presets[toSlot].name, newName.c_str());
    presets[toSlot].timestamp = millis();
    presets[toSlot].checksum = calculateChecksum(&presets[toSlot]);
    
    // Save to storage
    if (!savePresetToStorage(toSlot)) {
        failedOperations++;
        return PRESET_ERROR_STORAGE_FAILED;
    }
    
    successfulOperations++;
    logPresetOperation("copy", toSlot, PRESET_SUCCESS);
    
    return PRESET_SUCCESS;
}

PresetResult PresetManager::renamePreset(uint8_t slot, const char* newName) {
    totalOperations++;
    lastOperation = millis();
    
    if (slot >= MAX_PRESETS) {
        failedOperations++;
        return PRESET_ERROR_INVALID_SLOT;
    }
    
    if (!presets[slot].used) {
        failedOperations++;
        return PRESET_ERROR_NOT_FOUND;
    }
    
    if (!newName || strlen(newName) == 0 || strlen(newName) >= PRESET_NAME_LENGTH) {
        failedOperations++;
        return PRESET_ERROR_NAME_TOO_LONG;
    }
    
    // Update name and checksum
    strncpy(presets[slot].name, newName, PRESET_NAME_LENGTH - 1);
    presets[slot].name[PRESET_NAME_LENGTH - 1] = '\0';
    sanitizePresetName(presets[slot].name);
    presets[slot].checksum = calculateChecksum(&presets[slot]);
    
    // Save to storage
    if (!savePresetToStorage(slot)) {
        failedOperations++;
        return PRESET_ERROR_STORAGE_FAILED;
    }
    
    successfulOperations++;
    logPresetOperation("rename", slot, PRESET_SUCCESS);
    
    return PRESET_SUCCESS;
}

bool PresetManager::isSlotUsed(uint8_t slot) const {
    if (slot >= MAX_PRESETS) return false;
    return presets[slot].used;
}

String PresetManager::getPresetName(uint8_t slot) const {
    if (slot >= MAX_PRESETS || !presets[slot].used) {
        return String("");
    }
    return String(presets[slot].name);
}

uint32_t PresetManager::getPresetTimestamp(uint8_t slot) const {
    if (slot >= MAX_PRESETS || !presets[slot].used) {
        return 0;
    }
    return presets[slot].timestamp;
}

uint8_t PresetManager::getUsedSlotCount() const {
    uint8_t count = 0;
    for (int i = 0; i < MAX_PRESETS; i++) {
        if (presets[i].used) count++;
    }
    return count;
}

uint8_t PresetManager::getFreeSlotCount() const {
    return MAX_PRESETS - getUsedSlotCount();
}

int8_t PresetManager::findFreeSlot() const {
    for (int i = 0; i < MAX_PRESETS; i++) {
        if (!presets[i].used) {
            return i;
        }
    }
    return -1; // No free slots
}

PresetResult PresetManager::validateSlot(uint8_t slot) const {
    if (slot >= MAX_PRESETS) {
        return PRESET_ERROR_INVALID_SLOT;
    }
    
    if (!presets[slot].used) {
        return PRESET_ERROR_NOT_FOUND;
    }
    
    // Check checksum
    uint16_t calcChecksum = calculateChecksum(&presets[slot]);
    if (calcChecksum != presets[slot].checksum) {
        return PRESET_ERROR_CHECKSUM_FAILED;
    }
    
    // Validate structure
    if (!validatePreset(&presets[slot])) {
        return PRESET_ERROR_CORRUPTED;
    }
    
    return PRESET_SUCCESS;
}

PresetResult PresetManager::repairPreset(uint8_t slot) {
    if (slot >= MAX_PRESETS) {
        return PRESET_ERROR_INVALID_SLOT;
    }
    
    if (!presets[slot].used) {
        return PRESET_ERROR_NOT_FOUND;
    }
    
    // Try to reload from storage
    if (loadPresetFromStorage(slot)) {
        // Recalculate checksum
        presets[slot].checksum = calculateChecksum(&presets[slot]);
        
        // Re-save to ensure consistency
        if (savePresetToStorage(slot)) {
            Serial.print(F("PresetManager: Repaired slot "));
            Serial.println(slot);
            return PRESET_SUCCESS;
        }
    }
    
    return PRESET_ERROR_CORRUPTED;
}

uint8_t PresetManager::validateAllPresets() {
    uint8_t corruptedCount = 0;
    
    Serial.println(F("PresetManager: Validating all presets..."));
    
    for (int i = 0; i < MAX_PRESETS; i++) {
        if (presets[i].used) {
            PresetResult result = validateSlot(i);
            if (result != PRESET_SUCCESS) {
                corruptedCount++;
                Serial.print(F("PresetManager: Slot "));
                Serial.print(i);
                Serial.print(F(" is corrupted (error "));
                Serial.print((int)result);
                Serial.println(F(")"));
                
                // Attempt repair
                if (repairPreset(i) == PRESET_SUCCESS) {
                    corruptedCount--;
                    Serial.print(F("PresetManager: Slot "));
                    Serial.print(i);
                    Serial.println(F(" repaired"));
                }
            }
        }
    }
    
    Serial.print(F("PresetManager: Validation complete. "));
    Serial.print(corruptedCount);
    Serial.println(F(" corrupted presets found"));
    
    return corruptedCount;
}

void PresetManager::defragmentStorage() {
    Serial.println(F("PresetManager: Defragmenting storage..."));
    
    // Compact used presets to lower slot numbers
    uint8_t writeIndex = 0;
    ExtendedPreset tempPresets[MAX_PRESETS];
    
    // Copy used presets to temporary array
    for (int i = 0; i < MAX_PRESETS; i++) {
        if (presets[i].used && writeIndex < MAX_PRESETS) {
            memcpy(&tempPresets[writeIndex], &presets[i], sizeof(ExtendedPreset));
            writeIndex++;
        }
    }
    
    // Clear all presets
    clearAllPresets();
    
    // Copy back compacted presets
    for (int i = 0; i < writeIndex && i < MAX_PRESETS; i++) {
        memcpy(&presets[i], &tempPresets[i], sizeof(ExtendedPreset));
        savePresetToStorage(i);
    }
    
    Serial.print(F("PresetManager: Defragmentation complete. "));
    Serial.print(writeIndex);
    Serial.println(F(" presets compacted"));
}

void PresetManager::clearAllPresets() {
    Serial.println(F("PresetManager: Clearing all presets..."));
    
    for (int i = 0; i < MAX_PRESETS; i++) {
        if (presets[i].used) {
            deletePreset(i);
        }
    }
    
    Serial.println(F("PresetManager: All presets cleared"));
}

void PresetManager::printStatistics() const {
    Serial.println(F("=== PresetManager Statistics ==="));
    Serial.print(F("Total Operations: "));
    Serial.println(totalOperations);
    Serial.print(F("Successful Operations: "));
    Serial.println(successfulOperations);
    Serial.print(F("Failed Operations: "));
    Serial.println(failedOperations);
    Serial.print(F("Success Rate: "));
    Serial.print(getSuccessRate());
    Serial.println(F("%"));
    Serial.print(F("Used Slots: "));
    Serial.print(getUsedSlotCount());
    Serial.print(F("/"));
    Serial.println(MAX_PRESETS);
    Serial.print(F("Free Slots: "));
    Serial.println(getFreeSlotCount());
    Serial.print(F("Last Operation: "));
    Serial.print((millis() - lastOperation) / 1000);
    Serial.println(F(" seconds ago"));
    Serial.println(F("==============================="));
}

void PresetManager::printSlotInfo(uint8_t slot) const {
    if (slot >= MAX_PRESETS) {
        Serial.println(F("Invalid slot"));
        return;
    }
    
    Serial.print(F("Slot "));
    Serial.print(slot);
    Serial.print(F(": "));
    
    if (!presets[slot].used) {
        Serial.println(F("Empty"));
        return;
    }
    
    Serial.print(F("'"));
    Serial.print(presets[slot].name);
    Serial.print(F("' - "));
    Serial.print(patternNames[presets[slot].pattern]);
    Serial.print(F(" ("));
    Serial.print((millis() - presets[slot].timestamp) / 1000);
    Serial.print(F("s ago, v"));
    Serial.print(presets[slot].version);
    Serial.print(F(", checksum: "));
    Serial.print(presets[slot].checksum);
    Serial.println(F(")"));
}

void PresetManager::printAllPresets() const {
    Serial.println(F("=== All Presets ==="));
    for (int i = 0; i < MAX_PRESETS; i++) {
        printSlotInfo(i);
    }
    Serial.println(F("=================="));
}

uint32_t PresetManager::getSuccessRate() const {
    if (totalOperations == 0) return 100;
    return (successfulOperations * 100) / totalOperations;
}

void PresetManager::factoryReset() {
    Serial.println(F("PresetManager: Factory reset initiated..."));
    
    // Clear all presets from memory and storage
    clearAllPresets();
    
    // Reset statistics
    totalOperations = 0;
    successfulOperations = 0;
    failedOperations = 0;
    lastOperation = millis();
    
    Serial.println(F("PresetManager: Factory reset complete"));
}

// Private methods

uint16_t PresetManager::calculateChecksum(const ExtendedPreset* preset) const {
    if (!preset) return 0;
    
    uint16_t checksum = CHECKSUM_SEED;
    const uint8_t* data = (const uint8_t*)preset;
    
    // Calculate checksum for all fields except checksum itself
    size_t checksumOffset = offsetof(ExtendedPreset, checksum);
    size_t totalSize = sizeof(ExtendedPreset);
    
    // Hash data before checksum field
    for (size_t i = 0; i < checksumOffset; i++) {
        checksum += data[i];
        checksum = (checksum << 1) | (checksum >> 15); // Rotate left
    }
    
    // Hash data after checksum field
    for (size_t i = checksumOffset + sizeof(uint16_t); i < totalSize; i++) {
        checksum += data[i];
        checksum = (checksum << 1) | (checksum >> 15); // Rotate left
    }
    
    return checksum;
}

bool PresetManager::validatePreset(const ExtendedPreset* preset) const {
    if (!preset) return false;
    
    // Basic structure validation
    if (!preset->used) return false;
    if (preset->name[0] == '\0') return false;
    if (preset->version > PRESET_VERSION) return false;
    
    // Range validation
    if (preset->pattern >= NUM_PATTERNS) return false;
    if (preset->brightness == 0 || preset->brightness > 255) return false;
    if (preset->speed == 0) return false;
    if (preset->audioMode > AUDIO_ALL) return false;
    if (preset->mouthPattern >= NUM_MOUTH_PATTERNS) return false;
    if (preset->eyeMode > 4) return false;
    
    // Color index validation
    for (int i = 0; i < 9; i++) {
        if (preset->blockColors[i] >= NUM_STANDARD_COLORS && preset->blockColors[i] != 10) {
            return false; // 10 is allowed (random)
        }
    }
    
    // Timing validation
    if (preset->sideMinTime > preset->sideMaxTime) return false;
    if (preset->blockMinTime > preset->blockMaxTime) return false;
    
    return true;
}

void PresetManager::copyCurrentToPreset(ExtendedPreset* preset) const {
    if (!preset) return;
    
    // Core settings
    preset->pattern = currentPattern;
    preset->brightness = ledBrightness;
    preset->speed = effectSpeed;
    preset->fadeSpeed = fadeSpeed;
    
    // Colors
    preset->solidColor = solidColorIndex;
    preset->eyeColor1 = eyeColorIndex;
    preset->eyeColor2 = eyeColorIndex2;
    preset->mouthColor1 = mouthColorIndex;
    preset->mouthColor2 = mouthColorIndex2;
    
    // Copy block colors safely
    for (int i = 0; i < 9; i++) {
        preset->blockColors[i] = blockColors[i];
    }
    
    // Modes
    preset->eyeMode = eyeMode;
    preset->mouthPattern = mouthPattern;
    preset->mouthSplitMode = mouthSplitMode;
    preset->audioMode = audioMode;
    
    // Audio settings
    preset->audioSensitivity = audioSensitivity;
    preset->audioThreshold = audioThreshold;
    preset->audioAutoGain = audioAutoGain;
    
    // Brightness adjustments
    preset->eyeBrightness = eyeBrightness;
    preset->bodyBrightness = bodyBrightness;
    preset->mouthOuterBoost = mouthOuterBoost;
    preset->mouthInnerBoost = mouthInnerBoost;

    // Eye flicker control
    preset->eyeFlickerEnabled = eyeFlickerEnabled;
    preset->eyeFlickerMinTime = eyeFlickerMinTime;
    preset->eyeFlickerMaxTime = eyeFlickerMaxTime;
    preset->eyeStaticBrightness = eyeStaticBrightness;
    
    // Demo mode
    preset->demoMode = demoMode;
    preset->demoTime = demoTime;
    
    // Pattern-specific settings
    preset->flashColor = flashColorIndex;
    preset->flashSpeed = flashSpeed;
    preset->knightColor = knightColorIndex;
    preset->breathingColor = breathingColorIndex;
    preset->matrixColor = matrixColorIndex;
    preset->strobeColor = strobeColorIndex;
    
    // Side LED settings
    preset->sideColor1 = sideColor1;
    preset->sideColor2 = sideColor2;
    preset->sideColor3 = sideColor3;
    preset->sideColorMode = sideColorMode;
    preset->sideBlinkRate = sideBlinkRate;
    preset->blockBlinkRate = blockBlinkRate;
    
    // Timing settings
    preset->sideMinTime = sideMinTime;
    preset->sideMaxTime = sideMaxTime;
    preset->blockMinTime = blockMinTime;
    preset->blockMaxTime = blockMaxTime;
    
    // Mouth animation settings
    preset->talkSpeed = talkSpeed;
    preset->smileWidth = smileWidth;
    preset->waveSpeed = waveSpeed;
    preset->pulseSpeed = pulseSpeed;
    
    // Clear reserved bytes
    memset(preset->reserved, 0, sizeof(preset->reserved));
}

bool PresetManager::applyPresetToCurrent(const ExtendedPreset* preset) const {
    if (!preset || !validatePreset(preset)) {
        return false;
    }
    
    // Apply core settings
    currentPattern = preset->pattern;
    ledBrightness = preset->brightness;
    effectSpeed = preset->speed;
    fadeSpeed = preset->fadeSpeed;
    
    // Apply colors
    solidColorIndex = preset->solidColor;
    eyeColorIndex = preset->eyeColor1;
    eyeColorIndex2 = preset->eyeColor2;
    mouthColorIndex = preset->mouthColor1;
    mouthColorIndex2 = preset->mouthColor2;
    
    // Apply block colors safely
    for (int i = 0; i < 9; i++) {
        if (preset->blockColors[i] < NUM_STANDARD_COLORS || preset->blockColors[i] == 10) {
            blockColors[i] = preset->blockColors[i];
        }
    }
    
    // Apply modes
    eyeMode = preset->eyeMode;
    mouthPattern = preset->mouthPattern;
    mouthSplitMode = preset->mouthSplitMode;
    audioMode = preset->audioMode;
    
    // Apply audio settings
    audioSensitivity = preset->audioSensitivity;
    audioThreshold = preset->audioThreshold;
    audioAutoGain = preset->audioAutoGain;
    
    // Apply brightness adjustments
    eyeBrightness = preset->eyeBrightness;
    bodyBrightness = preset->bodyBrightness;
    mouthOuterBoost = preset->mouthOuterBoost;
    mouthInnerBoost = preset->mouthInnerBoost;

    // Eye flicker control
    eyeFlickerEnabled = preset->eyeFlickerEnabled;
    eyeFlickerMinTime = preset->eyeFlickerMinTime;
    eyeFlickerMaxTime = preset->eyeFlickerMaxTime;
    eyeStaticBrightness = preset->eyeStaticBrightness;
    
    // Apply demo mode
    demoMode = preset->demoMode;
    demoTime = preset->demoTime;
    
    // Apply pattern-specific settings
    flashColorIndex = preset->flashColor;
    flashSpeed = preset->flashSpeed;
    knightColorIndex = preset->knightColor;
    breathingColorIndex = preset->breathingColor;
    matrixColorIndex = preset->matrixColor;
    strobeColorIndex = preset->strobeColor;
    
    // Apply side LED settings
    sideColor1 = preset->sideColor1;
    sideColor2 = preset->sideColor2;
    sideColor3 = preset->sideColor3;
    sideColorMode = preset->sideColorMode;
    sideBlinkRate = preset->sideBlinkRate;
    blockBlinkRate = preset->blockBlinkRate;
    
    // Apply timing settings
    sideMinTime = preset->sideMinTime;
    sideMaxTime = preset->sideMaxTime;
    blockMinTime = preset->blockMinTime;
    blockMaxTime = preset->blockMaxTime;
    
    // Apply mouth animation settings
    talkSpeed = preset->talkSpeed;
    smileWidth = preset->smileWidth;
    waveSpeed = preset->waveSpeed;
    pulseSpeed = preset->pulseSpeed;
    
    // Apply brightness immediately to FastLED
    FastLED.setBrightness(ledBrightness);
    
    return true;
}

void PresetManager::sanitizePresetName(char* name) const {
    if (!name) return;
    
    size_t len = strlen(name);
    for (size_t i = 0; i < len; i++) {
        // Replace invalid characters with underscore
        if (name[i] < 32 || name[i] > 126) {
            name[i] = '_';
        }
    }
}

bool PresetManager::loadPresetFromStorage(uint8_t slot) {
    if (slot >= MAX_PRESETS) return false;
    
    String storageKey = String(STORAGE_PREFIX) + String(slot);
    prefs.begin(storageKey.c_str(), true); // Read-only
    
    bool success = false;
    presets[slot].used = prefs.getBool("used", false);
    
    if (presets[slot].used) {
        // Load all preset data
        prefs.getString("name", presets[slot].name, PRESET_NAME_LENGTH);
        presets[slot].timestamp = prefs.getULong("timestamp", 0);
        presets[slot].version = prefs.getUChar("version", 1);
        
        // Load settings with validation
        presets[slot].pattern = prefs.getUChar("pattern", 0);
        presets[slot].brightness = prefs.getUChar("brightness", 90);
        presets[slot].speed = prefs.getUChar("speed", 128);
        presets[slot].fadeSpeed = prefs.getUChar("fadeSpeed", 8);
        
        // Load colors
        presets[slot].solidColor = prefs.getUChar("solidColor", 0);
        presets[slot].eyeColor1 = prefs.getUChar("eyeColor1", 3);
        presets[slot].eyeColor2 = prefs.getUChar("eyeColor2", 2);
        presets[slot].mouthColor1 = prefs.getUChar("mouthColor1", 2);
        presets[slot].mouthColor2 = prefs.getUChar("mouthColor2", 0);
        
        // Load block colors with error checking
        size_t len = prefs.getBytes("blockColors", presets[slot].blockColors, 9);
        if (len != 9) {
            // Initialize with defaults if corrupted
            for (int i = 0; i < 9; i++) {
                presets[slot].blockColors[i] = i % NUM_STANDARD_COLORS;
            }
        }
        
        // Load modes
        presets[slot].eyeMode = prefs.getUChar("eyeMode", 0);
        presets[slot].mouthPattern = prefs.getUChar("mouthPattern", 1);
        presets[slot].mouthSplitMode = prefs.getUChar("mouthSplitMode", 0);
        presets[slot].audioMode = prefs.getUChar("audioMode", 0);
        
        // Load audio settings
        presets[slot].audioSensitivity = prefs.getUChar("audioSens", 5);
        presets[slot].audioThreshold = prefs.getUShort("audioThresh", 100);
        presets[slot].audioAutoGain = prefs.getBool("audioAutoGain", true);
        
        // Load brightness settings
        presets[slot].eyeBrightness = prefs.getUChar("eyeBright", 125);
        presets[slot].bodyBrightness = prefs.getUChar("bodyBright", 100);
        presets[slot].mouthOuterBoost = prefs.getUChar("mouthOuter", 100);
        presets[slot].mouthInnerBoost = prefs.getUChar("mouthInner", 150);

        // Load eye flicker settings
    presets[slot].eyeFlickerEnabled = prefs.getBool("eyeFlickerEn", true);
    presets[slot].eyeFlickerMinTime = prefs.getUShort("eyeFlickMin", 200);
    presets[slot].eyeFlickerMaxTime = prefs.getUShort("eyeFlickMax", 1600);
    presets[slot].eyeStaticBrightness = prefs.getUChar("eyeStaticBr", 255);
        
        // Load demo mode
        presets[slot].demoMode = prefs.getBool("demoMode", false);
        presets[slot].demoTime = prefs.getUShort("demoTime", 10);
        
        // Load pattern-specific settings
        presets[slot].flashColor = prefs.getUChar("flashColor", 0);
        presets[slot].flashSpeed = prefs.getUChar("flashSpeed", 5);
        presets[slot].knightColor = prefs.getUChar("knightColor", 0);
        presets[slot].breathingColor = prefs.getUChar("breathColor", 2);
        presets[slot].matrixColor = prefs.getUChar("matrixColor", 11);
        presets[slot].strobeColor = prefs.getUChar("strobeColor", 3);
        
        // Load side LED settings
        presets[slot].sideColor1 = prefs.getUChar("sideColor1", 0);
        presets[slot].sideColor2 = prefs.getUChar("sideColor2", 2);
        presets[slot].sideColor3 = prefs.getUChar("sideColor3", 3);
        presets[slot].sideColorMode = prefs.getUChar("sideMode", 0);
        presets[slot].sideBlinkRate = prefs.getUChar("sideRate", 128);
        presets[slot].blockBlinkRate = prefs.getUChar("blockRate", 128);
        
        // Load timing settings
        presets[slot].sideMinTime = prefs.getUShort("sideMin", 500);
        presets[slot].sideMaxTime = prefs.getUShort("sideMax", 2500);
        presets[slot].blockMinTime = prefs.getUShort("blockMin", 200);
        presets[slot].blockMaxTime = prefs.getUShort("blockMax", 1500);
        
        // Load mouth animation settings
        presets[slot].talkSpeed = prefs.getUChar("talkSpeed", 5);
        presets[slot].smileWidth = prefs.getUChar("smileWidth", 6);
        presets[slot].waveSpeed = prefs.getUChar("waveSpeed", 5);
        presets[slot].pulseSpeed = prefs.getUChar("pulseSpeed", 5);
        
        // Load checksum (must be last)
        presets[slot].checksum = prefs.getUShort("checksum", 0);
        
        success = true;
    }
    
    prefs.end();
    return success;
}

bool PresetManager::savePresetToStorage(uint8_t slot) {
    if (slot >= MAX_PRESETS) return false;
    
    String storageKey = String(STORAGE_PREFIX) + String(slot);
    prefs.begin(storageKey.c_str(), false); // Read-write
    
    const ExtendedPreset* p = &presets[slot];
    
    // Save basic metadata
    prefs.putBool("used", p->used);
    prefs.putString("name", p->name);
    prefs.putULong("timestamp", p->timestamp);
    prefs.putUChar("version", p->version);
    
    // Save core settings
    prefs.putUChar("pattern", p->pattern);
    prefs.putUChar("brightness", p->brightness);
    prefs.putUChar("speed", p->speed);
    prefs.putUChar("fadeSpeed", p->fadeSpeed);
    
    // Save colors
    prefs.putUChar("solidColor", p->solidColor);
    prefs.putUChar("eyeColor1", p->eyeColor1);
    prefs.putUChar("eyeColor2", p->eyeColor2);
    prefs.putUChar("mouthColor1", p->mouthColor1);
    prefs.putUChar("mouthColor2", p->mouthColor2);
    prefs.putBytes("blockColors", p->blockColors, 9);
    
    // Save modes
    prefs.putUChar("eyeMode", p->eyeMode);
    prefs.putUChar("mouthPattern", p->mouthPattern);
    prefs.putUChar("mouthSplitMode", p->mouthSplitMode);
    prefs.putUChar("audioMode", p->audioMode);
    
    // Save audio settings
    prefs.putUChar("audioSens", p->audioSensitivity);
    prefs.putUShort("audioThresh", p->audioThreshold);
    prefs.putBool("audioAutoGain", p->audioAutoGain);
    
    // Save brightness settings
    prefs.putUChar("eyeBright", p->eyeBrightness);
    prefs.putUChar("bodyBright", p->bodyBrightness);
    prefs.putUChar("mouthOuter", p->mouthOuterBoost);
    prefs.putUChar("mouthInner", p->mouthInnerBoost);

    // Save eye flicker settings
    prefs.putBool("eyeFlickerEn", p->eyeFlickerEnabled);
    prefs.putUShort("eyeFlickMin", p->eyeFlickerMinTime);
    prefs.putUShort("eyeFlickMax", p->eyeFlickerMaxTime);
    prefs.putUChar("eyeStaticBr", p->eyeStaticBrightness);
    
    // Save demo mode
    prefs.putBool("demoMode", p->demoMode);
    prefs.putUShort("demoTime", p->demoTime);
    
    // Save pattern-specific settings
    prefs.putUChar("flashColor", p->flashColor);
    prefs.putUChar("flashSpeed", p->flashSpeed);
    prefs.putUChar("knightColor", p->knightColor);
    prefs.putUChar("breathColor", p->breathingColor);
    prefs.putUChar("matrixColor", p->matrixColor);
    prefs.putUChar("strobeColor", p->strobeColor);
    
    // Save side LED settings
    prefs.putUChar("sideColor1", p->sideColor1);
    prefs.putUChar("sideColor2", p->sideColor2);
    prefs.putUChar("sideColor3", p->sideColor3);
    prefs.putUChar("sideMode", p->sideColorMode);
    prefs.putUChar("sideRate", p->sideBlinkRate);
    prefs.putUChar("blockRate", p->blockBlinkRate);
    
    // Save timing settings
    prefs.putUShort("sideMin", p->sideMinTime);
    prefs.putUShort("sideMax", p->sideMaxTime);
    prefs.putUShort("blockMin", p->blockMinTime);
    prefs.putUShort("blockMax", p->blockMaxTime);
    
    // Save mouth animation settings
    prefs.putUChar("talkSpeed", p->talkSpeed);
    prefs.putUChar("smileWidth", p->smileWidth);
    prefs.putUChar("waveSpeed", p->waveSpeed);
    prefs.putUChar("pulseSpeed", p->pulseSpeed);
    
    // Save checksum (must be last)
    prefs.putUShort("checksum", p->checksum);
    
    prefs.end();
    return true;
}

void PresetManager::initializePresetDefaults(ExtendedPreset* preset) const {
    if (!preset) return;
    
    memset(preset, 0, sizeof(ExtendedPreset));
    preset->used = false;
    preset->version = PRESET_VERSION;
    preset->timestamp = millis();
    
    // Set reasonable defaults
    preset->brightness = 90;
    preset->speed = 128;
    preset->fadeSpeed = 8;
    preset->eyeColor1 = 3; // White
    preset->eyeColor2 = 2; // Blue
    preset->mouthColor1 = 2; // Blue
    preset->audioSensitivity = 5;
    preset->audioThreshold = 100;
    preset->audioAutoGain = true;
    preset->eyeBrightness = 125;
    preset->bodyBrightness = 100;
    preset->mouthOuterBoost = 100;
    preset->mouthInnerBoost = 150;
    preset->eyeFlickerEnabled = true;
    preset->eyeFlickerMinTime = 200;
    preset->eyeFlickerMaxTime = 1600;
    preset->eyeStaticBrightness = 255;
    preset->demoTime = 10;
    preset->flashSpeed = 5;
    preset->breathingColor = 2;
    preset->matrixColor = 11;
    preset->strobeColor = 3;
    preset->sideBlinkRate = 128;
    preset->blockBlinkRate = 128;
    preset->sideMinTime = 500;
    preset->sideMaxTime = 2500;
    preset->blockMinTime = 200;
    preset->blockMaxTime = 1500;
    preset->talkSpeed = 5;
    preset->smileWidth = 6;
    preset->waveSpeed = 5;
    preset->pulseSpeed = 5;
}

void PresetManager::logPresetOperation(const char* operation, uint8_t slot, PresetResult result) const {
    Serial.print(F("PresetManager: "));
    Serial.print(operation);
    Serial.print(F(" slot "));
    Serial.print(slot);
    
    if (result == PRESET_SUCCESS) {
        Serial.println(F(" - SUCCESS"));
    } else {
        Serial.print(F(" - ERROR "));
        Serial.println((int)result);
    }
}