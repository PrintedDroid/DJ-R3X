#ifndef SETTINGS_H
#define SETTINGS_H

#include "config.h"
#include "globals.h"

void initSettings();
void loadSettings();
void saveSettings();
void resetToDefaults();

// v5.0.1: Legacy preset functions (slots 1-3 only)
// These now map to v5 PresetManager for compatibility
// New code should use PresetManager directly (10 slots, named presets)
void saveUserPreset(uint8_t presetNum);    // Maps to PresetManager slots 1-3
bool loadUserPreset(uint8_t presetNum);    // Maps to PresetManager slots 1-3
void deleteUserPreset(uint8_t presetNum);  // Maps to PresetManager slots 1-3

#endif