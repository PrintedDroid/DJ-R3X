#ifndef PRESET_MANAGER_H
#define PRESET_MANAGER_H

#include "config.h"
#include <Preferences.h>

/*
// Only include ArduinoJson if JSON export is enabled
#ifdef ENABLE_JSON_EXPORT
#include <ArduinoJson.h>
#endif
*/

// Preset validation results
enum PresetResult {
    PRESET_SUCCESS = 0,
    PRESET_ERROR_INVALID_SLOT,
    PRESET_ERROR_NOT_FOUND,
    PRESET_ERROR_CHECKSUM_FAILED,
    PRESET_ERROR_STORAGE_FAILED,
    PRESET_ERROR_NAME_TOO_LONG,
    PRESET_ERROR_CORRUPTED,
    PRESET_ERROR_NO_FREE_SLOTS
};

// Extended preset structure with all DJ Rex settings
struct ExtendedPreset {
    char name[PRESET_NAME_LENGTH];
    uint32_t timestamp;
    uint16_t checksum;
    bool used;
    uint8_t version;  // For future compatibility
    
    // Core pattern settings
    uint8_t pattern;
    uint8_t brightness;
    uint8_t speed;
    uint8_t fadeSpeed;
    
    // Color settings
    uint8_t solidColor;
    uint8_t eyeColor1;
    uint8_t eyeColor2;
    uint8_t mouthColor1;
    uint8_t mouthColor2;
    uint8_t blockColors[9];
    
    // Mode settings
    uint8_t eyeMode;
    uint8_t mouthPattern;
    uint8_t mouthSplitMode;
    uint8_t audioMode;
    
    // Audio settings
    uint8_t audioSensitivity;
    uint16_t audioThreshold;
    bool audioAutoGain;
    
    // Brightness adjustments
    uint8_t eyeBrightness;
    uint8_t bodyBrightness;
    uint8_t mouthOuterBoost;
    uint8_t mouthInnerBoost;

    // Eye flicker control
    bool eyeFlickerEnabled;
    uint16_t eyeFlickerMinTime;
    uint16_t eyeFlickerMaxTime;
    uint8_t eyeStaticBrightness;
    
    // Demo mode
    bool demoMode;
    uint16_t demoTime;
    
    // Pattern-specific settings
    uint8_t flashColor;
    uint8_t flashSpeed;
    uint8_t knightColor;
    uint8_t breathingColor;
    uint8_t matrixColor;
    uint8_t strobeColor;
    
    // Side LED settings
    uint8_t sideColor1;
    uint8_t sideColor2;
    uint8_t sideColor3;
    uint8_t sideColorMode;
    uint8_t sideBlinkRate;
    uint8_t blockBlinkRate;
    
    // Timing settings
    uint16_t sideMinTime;
    uint16_t sideMaxTime;
    uint16_t blockMinTime;
    uint16_t blockMaxTime;
    
    // Mouth animation settings
    uint8_t talkSpeed;
    uint8_t smileWidth;
    uint8_t waveSpeed;
    uint8_t pulseSpeed;
    
    // Reserved for future expansion
    uint8_t reserved[8];
};

class PresetManager {
private:
    ExtendedPreset presets[MAX_PRESETS];
    Preferences prefs;  // Stack allocation - NO MEMORY LEAK!
    
    // Internal state
    bool initialized;
    uint32_t totalOperations;
    uint32_t successfulOperations;
    uint32_t failedOperations;
    unsigned long lastOperation;
    
    // Internal methods
    uint16_t calculateChecksum(const ExtendedPreset* preset) const;
    bool validatePreset(const ExtendedPreset* preset) const;
    void copyCurrentToPreset(ExtendedPreset* preset) const;
    bool applyPresetToCurrent(const ExtendedPreset* preset) const;
    void sanitizePresetName(char* name) const;
    bool loadPresetFromStorage(uint8_t slot);
    bool savePresetToStorage(uint8_t slot);
    void initializePresetDefaults(ExtendedPreset* preset) const;
    void logPresetOperation(const char* operation, uint8_t slot, PresetResult result) const;

public:
    PresetManager();
    ~PresetManager();  // Proper destructor
    
    // Initialization
    void begin();
    bool isInitialized() const { return initialized; }
    
    // Main preset operations
    PresetResult savePreset(uint8_t slot, const char* name);
    PresetResult loadPreset(uint8_t slot);
    PresetResult deletePreset(uint8_t slot);
    PresetResult copyPreset(uint8_t fromSlot, uint8_t toSlot);
    PresetResult renamePreset(uint8_t slot, const char* newName);
    
    // Query operations
    bool isSlotUsed(uint8_t slot) const;
    String getPresetName(uint8_t slot) const;
    uint32_t getPresetTimestamp(uint8_t slot) const;
    uint8_t getUsedSlotCount() const;
    uint8_t getFreeSlotCount() const;
    int8_t findFreeSlot() const;
    
    // Validation and maintenance
    PresetResult validateSlot(uint8_t slot) const;
    PresetResult repairPreset(uint8_t slot);
    uint8_t validateAllPresets();  // Returns count of corrupted presets
    void defragmentStorage();      // Clean up storage
    
    // Bulk operations
    PresetResult exportAllPresets(String& output) const;
    PresetResult importAllPresets(const String& input);
    void clearAllPresets();
    
    // Statistics and diagnostics
    void printStatistics() const;
    void printSlotInfo(uint8_t slot) const;
    void printAllPresets() const;
    uint32_t getTotalOperations() const { return totalOperations; }
    uint32_t getSuccessRate() const;
    
/*
    // Advanced features (if JSON enabled)
    #ifdef ENABLE_JSON_EXPORT
    String getPresetListJSON() const;
    PresetResult exportPresetJSON(uint8_t slot, StaticJsonDocument<2048>& doc) const;
    PresetResult importPresetJSON(const StaticJsonDocument<2048>& doc);
    #endif
*/
    
    // Factory reset
    void factoryReset();
    
private:
    // Constants for validation
    static const uint16_t PRESET_VERSION = 1;
    static const uint16_t CHECKSUM_SEED = 0xA5C3;
    static const char STORAGE_PREFIX[];
};

extern PresetManager presetManager;

// Thread-safe LED operations
bool acquireLEDMutex(uint32_t timeoutMs, const char* caller);
void releaseLEDMutex(const char* caller);

// LED performance monitoring
void printLEDPerformanceStats();
void resetLEDPerformanceStats();
bool isLEDOperationSafe();

// System validation
void validateHelperState();

// Emergency functions
void emergencyLEDClear();

#endif