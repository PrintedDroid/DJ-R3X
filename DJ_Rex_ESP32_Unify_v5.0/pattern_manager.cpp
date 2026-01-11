// pattern_manager.cpp - v5.0 Centralized Pattern Management
#include "pattern_manager.h"
#include "event_logger.h"
#include <Arduino.h>

PatternManager patternManager;

// Pattern information table (matches pattern order in gPatterns)
const PatternInfo PatternManager::patternInfos[] = {
    {"Off",              CAT_OFF,      false},  // 0
    {"Random Blocks",    CAT_ANIMATED, false},  // 1
    {"Solid Color",      CAT_STATIC,   false},  // 2
    {"Short Circuit",    CAT_SPECIAL,  false},  // 3
    {"Confetti R/W",     CAT_ANIMATED, false},  // 4
    {"Rainbow",          CAT_ANIMATED, false},  // 5
    {"Rainbow Glitter",  CAT_ANIMATED, false},  // 6
    {"Confetti",         CAT_ANIMATED, false},  // 7
    {"Juggle",           CAT_ANIMATED, false},  // 8
    {"Audio Sync",       CAT_AUDIO,    true},   // 9
    {"Solid Flash",      CAT_ANIMATED, false},  // 10
    {"Knight Rider",     CAT_ANIMATED, false},  // 11
    {"Breathing",        CAT_ANIMATED, false},  // 12
    {"Matrix Rain",      CAT_ANIMATED, false},  // 13
    {"Strobe",           CAT_ANIMATED, false},  // 14
    {"Audio VU Meter",   CAT_AUDIO,    true},   // 15
    {"Custom Blocks",    CAT_ANIMATED, false},  // 16
    {"Plasma",           CAT_ANIMATED, false},  // 17
    {"Fire",             CAT_ANIMATED, false},  // 18
    {"Twinkle",          CAT_ANIMATED, false},  // 19
};

void PatternManager::begin() {
    Serial.println(F("Pattern Manager initialized"));
    Serial.print(F("Available patterns: "));
    Serial.println(getPatternCount());
}

void PatternManager::setPattern(uint8_t pattern) {
    if (pattern < getPatternCount()) {
        currentPattern = pattern;
        eventLogger.log(EVENT_PATTERN_CHANGE, pattern);
        Serial.print(F("Pattern set to: "));
        Serial.println(getPatternName(pattern));
    }
}

void PatternManager::nextPattern() {
    uint8_t next = (currentPattern + 1) % getPatternCount();
    setPattern(next);
}

void PatternManager::prevPattern() {
    uint8_t prev = (currentPattern == 0) ? getPatternCount() - 1 : currentPattern - 1;
    setPattern(prev);
}

uint8_t PatternManager::getCurrentPattern() {
    return currentPattern;
}

const char* PatternManager::getPatternName(uint8_t pattern) {
    if (pattern < getPatternCount()) {
        return patternInfos[pattern].name;
    }
    return "Unknown";
}

PatternCategory PatternManager::getPatternCategory(uint8_t pattern) {
    if (pattern < getPatternCount()) {
        return patternInfos[pattern].category;
    }
    return CAT_OFF;
}

uint8_t PatternManager::getPatternCount() {
    return NUM_PATTERNS;
}

uint8_t PatternManager::getNextInCategory(PatternCategory cat, uint8_t current) {
    for (uint8_t i = 1; i <= getPatternCount(); i++) {
        uint8_t idx = (current + i) % getPatternCount();
        if (patternInfos[idx].category == cat) {
            return idx;
        }
    }
    return current;
}

bool PatternManager::isAudioPattern(uint8_t pattern) {
    if (pattern < getPatternCount()) {
        return patternInfos[pattern].audioRequired;
    }
    return false;
}

uint8_t PatternManager::getRandomPattern(bool excludeCurrent) {
    uint8_t pattern;
    do {
        pattern = random8(1, getPatternCount()); // Skip 0 (Off)
    } while (excludeCurrent && pattern == currentPattern);
    return pattern;
}

uint8_t PatternManager::getRandomAnimatedPattern() {
    uint8_t attempts = 0;
    uint8_t pattern;
    do {
        pattern = random8(1, getPatternCount());
        attempts++;
    } while (patternInfos[pattern].category != CAT_ANIMATED && attempts < 50);
    return pattern;
}

uint8_t PatternManager::getRandomAudioPattern() {
    // Only patterns 9 and 15 are audio patterns
    return random8(2) == 0 ? 9 : 15;
}
