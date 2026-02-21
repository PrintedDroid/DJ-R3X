// pattern_manager.h - v5.0 Centralized Pattern Management
#ifndef PATTERN_MANAGER_H
#define PATTERN_MANAGER_H

#include "config.h"
#include "globals.h"

// Pattern categories
enum PatternCategory {
    CAT_OFF = 0,
    CAT_STATIC,
    CAT_ANIMATED,
    CAT_AUDIO,
    CAT_SPECIAL
};

// Pattern info structure
struct PatternInfo {
    const char* name;
    PatternCategory category;
    bool audioRequired;
};

class PatternManager {
public:
    void begin();

    // Pattern control
    void setPattern(uint8_t pattern);
    void nextPattern();
    void prevPattern();
    uint8_t getCurrentPattern();
    const char* getPatternName(uint8_t pattern);
    PatternCategory getPatternCategory(uint8_t pattern);

    // Pattern queries
    uint8_t getPatternCount();
    uint8_t getNextInCategory(PatternCategory cat, uint8_t current);
    bool isAudioPattern(uint8_t pattern);

    // Random pattern selection
    uint8_t getRandomPattern(bool excludeCurrent = true);
    uint8_t getRandomAnimatedPattern();
    uint8_t getRandomAudioPattern();

private:
    static const PatternInfo patternInfos[];
};

extern PatternManager patternManager;

#endif
