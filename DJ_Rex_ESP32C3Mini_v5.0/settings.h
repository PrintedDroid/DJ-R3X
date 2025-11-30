#ifndef SETTINGS_H
#define SETTINGS_H

#include "config.h"
#include "globals.h"

void initSettings();
void loadSettings();
void saveSettings();
void resetToDefaults();
void saveUserPreset(uint8_t presetNum);
bool loadUserPreset(uint8_t presetNum);
void deleteUserPreset(uint8_t presetNum);

#endif