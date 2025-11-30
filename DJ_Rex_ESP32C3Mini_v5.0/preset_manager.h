// preset_manager.h - v5.0 Extended Preset Manager (10 slots)
#ifndef PRESET_MANAGER_H
#define PRESET_MANAGER_H

#include "config.h"
#include "globals.h"

struct ExtendedPreset {
    char name[PRESET_NAME_LENGTH];
    uint8_t pattern;
    uint8_t brightness;
    uint8_t speed;
    uint8_t solidColor;
    uint8_t eyeColor;
    uint8_t eyeColor2;
    uint8_t eyeMode;
    uint8_t mouthPattern;
    uint8_t mouthColor;
    uint8_t mouthColor2;
    uint8_t mouthSplitMode;
    uint8_t audioMode;
    uint8_t blockColors[9];
    uint8_t sideColors[3];
    uint8_t sideMode;
    bool valid;
};

class PresetManager {
public:
    void begin();
    bool savePreset(uint8_t slot, const char* name = nullptr);
    bool loadPreset(uint8_t slot);
    bool deletePreset(uint8_t slot);
    void listPresets();
    bool isValidSlot(uint8_t slot);
    const char* getPresetName(uint8_t slot);

private:
    ExtendedPreset presets[MAX_PRESETS];
    void loadFromFlash();
    void saveToFlash(uint8_t slot);
    void applyPreset(const ExtendedPreset& preset);
    void captureCurrentState(ExtendedPreset& preset);
};

extern PresetManager presetManager;

#endif
