#ifndef PATTERN_MANAGER_H
#define PATTERN_MANAGER_H

#include "config.h"
#include "globals.h"

// Transition types for pattern changes
enum TransitionType {
    TRANSITION_NONE = 0,
    TRANSITION_FADE = 1,
    TRANSITION_WIPE = 2,
    TRANSITION_FLASH = 3
};

// Pattern change validation results
enum PatternChangeResult {
    PATTERN_CHANGE_SUCCESS = 0,
    PATTERN_CHANGE_INVALID_INDEX,
    PATTERN_CHANGE_MUTEX_TIMEOUT,
    PATTERN_CHANGE_IN_PROGRESS,
    PATTERN_CHANGE_SAME_PATTERN
};

class PatternManager {
private:
    // State management
    bool transitioning;
    bool patternChangeInProgress;
    unsigned long transitionStartTime;
    unsigned long lastPatternChange;
    
    // Transition parameters
    uint16_t transitionDuration;
    uint8_t fromPattern;
    uint8_t toPattern;
    TransitionType transitionType;
    uint8_t transitionProgress;
    
    // Safety and validation
    uint32_t patternChangeCount;
    unsigned long lastValidationTime;
    bool validationEnabled;

public:
    PatternManager();
    void begin();
    void update();
    
    // Pattern control functions
    PatternChangeResult changePattern(uint8_t newPattern, TransitionType transition = TRANSITION_FADE);
    PatternChangeResult nextPattern();
    PatternChangeResult previousPattern();
    PatternChangeResult setPatternSafe(uint8_t newPattern);
    
    // Query functions
    bool isTransitioning() const { return transitioning; }
    bool isChanging() const { return patternChangeInProgress; }
    uint8_t getCurrentPattern() const { return currentPattern; }
    uint8_t getTransitionProgress() const { return transitionProgress; }
    uint32_t getChangeCount() const { return patternChangeCount; }
    
    // Settings
    void setTransitionDuration(uint16_t duration);
    void setDefaultTransition(TransitionType type);
    void enableValidation(bool enabled) { validationEnabled = enabled; }
    
    // Status and diagnostics
    bool validateCurrentState();
    void printStatus();
    unsigned long getTimeSinceLastChange() const;

private:
    // Internal transition management
    void startTransition(uint8_t newPattern, TransitionType type);
    void updateTransition();
    void completeTransition();
    void applyTransitionEffect();
    
    // Safety and validation
    bool validatePatternIndex(uint8_t pattern) const;
    bool canChangePattern() const;
    void logPatternChange(uint8_t fromPattern, uint8_t toPattern, const char* reason = nullptr);
    
    // Thread safety helpers
    bool acquireLEDMutex(uint32_t timeoutMs = 200) const;
    void releaseLEDMutex() const;
};

extern PatternManager patternManager;

#endif