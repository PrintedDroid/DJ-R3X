#include "pattern_manager.h"
#include "event_logger.h"
#include "helpers.h"

PatternManager patternManager;

PatternManager::PatternManager() {
    transitioning = false;
    patternChangeInProgress = false;
    transitionStartTime = 0;
    lastPatternChange = 0;
    transitionDuration = 1000; // Default 1 second transition
    fromPattern = 0;
    toPattern = 0;
    transitionType = TRANSITION_FADE;
    transitionProgress = 0;
    patternChangeCount = 0;
    lastValidationTime = 0;
    validationEnabled = true;
}

void PatternManager::begin() {
    Serial.println(F("PatternManager: Initializing..."));
    
    // Initialize pattern manager
    transitioning = false;
    patternChangeInProgress = false;
    transitionProgress = 0;
    lastPatternChange = millis();
    patternChangeCount = 0;
    
    // Validate initial state
    if (!validatePatternIndex(currentPattern)) {
        Serial.println(F("PatternManager: Invalid initial pattern, resetting to 16"));
        currentPattern = 16; // Default to Custom Block Sequence
    }
    
    Serial.print(F("PatternManager: Started with pattern "));
    Serial.print(currentPattern);
    Serial.print(F(" - "));
    Serial.println(patternNames[currentPattern]);
}

void PatternManager::update() {
    if (transitioning) {
        updateTransition();
    }
    
    // Periodic validation (every 10 seconds)
    if (validationEnabled && millis() - lastValidationTime > 10000) {
        validateCurrentState();
        lastValidationTime = millis();
    }
}

PatternChangeResult PatternManager::changePattern(uint8_t newPattern, TransitionType transition) {
    // Input validation
    if (!validatePatternIndex(newPattern)) {
        Serial.print(F("PatternManager: Invalid pattern index: "));
        Serial.println(newPattern);
        return PATTERN_CHANGE_INVALID_INDEX;
    }
    
    // Check if already on this pattern
    if (newPattern == currentPattern && !transitioning) {
        return PATTERN_CHANGE_SAME_PATTERN;
    }
    
    // Check if change is allowed
    if (!canChangePattern()) {
        Serial.println(F("PatternManager: Pattern change blocked - operation in progress"));
        return PATTERN_CHANGE_IN_PROGRESS;
    }
    
    // Thread-safe pattern change
    if (!acquireLEDMutex(200)) {
        Serial.println(F("PatternManager: Could not acquire LED mutex"));
        return PATTERN_CHANGE_MUTEX_TIMEOUT;
    }
    
    // Perform the change
    uint8_t oldPattern = currentPattern;
    
    if (transition == TRANSITION_NONE) {
        // Immediate change
        currentPattern = newPattern;
        lastPatternChange = millis();
        patternChangeCount++;
    } else {
        // Transition change
        startTransition(newPattern, transition);
    }
    
    releaseLEDMutex();
    
    // Log the change
    logPatternChange(oldPattern, newPattern);
    
    // Event logging
    eventLogger.log(EVENT_PATTERN_CHANGE, 
                   String("Pattern: ") + String(newPattern) + 
                   " (" + patternNames[newPattern] + ")");
    
    return PATTERN_CHANGE_SUCCESS;
}

PatternChangeResult PatternManager::nextPattern() {
    uint8_t next = currentPattern + 1;
    if (next >= NUM_PATTERNS) {
        next = 1; // Skip pattern 0 (LEDs Off) in normal navigation
    }
    
    return changePattern(next, TRANSITION_FADE);
}

PatternChangeResult PatternManager::previousPattern() {
    uint8_t prev;
    if (currentPattern <= 1) {
        prev = NUM_PATTERNS - 1; // Wrap to last pattern
    } else {
        prev = currentPattern - 1;
        if (prev == 0) { // Skip pattern 0 (LEDs Off)
            prev = NUM_PATTERNS - 1;
        }
    }
    
    return changePattern(prev, TRANSITION_FADE);
}

PatternChangeResult PatternManager::setPatternSafe(uint8_t newPattern) {
    // Extra safe version with extended validation
    if (!validatePatternIndex(newPattern)) {
        return PATTERN_CHANGE_INVALID_INDEX;
    }
    
    // Check system state
    if (patternUpdating) {
        Serial.println(F("PatternManager: Pattern update in progress, deferring change"));
        // Could implement a deferred change queue here
        return PATTERN_CHANGE_IN_PROGRESS;
    }
    
    return changePattern(newPattern, TRANSITION_NONE);
}

void PatternManager::setTransitionDuration(uint16_t duration) {
    if (duration < 100) duration = 100;     // Minimum 100ms
    if (duration > 10000) duration = 10000; // Maximum 10 seconds
    
    transitionDuration = duration;
    Serial.print(F("PatternManager: Transition duration set to "));
    Serial.print(duration);
    Serial.println(F("ms"));
}

void PatternManager::setDefaultTransition(TransitionType type) {
    if (type >= 0 && type <= TRANSITION_FLASH) {
        transitionType = type;
        Serial.print(F("PatternManager: Default transition set to "));
        Serial.println((int)type);
    }
}

bool PatternManager::validateCurrentState() {
    bool isValid = true;
    
    // Check pattern index bounds
    if (!validatePatternIndex(currentPattern)) {
        Serial.print(F("PatternManager: Invalid current pattern: "));
        Serial.println(currentPattern);
        currentPattern = 16; // Safe default
        isValid = false;
    }
    
    // Check for stuck transitions
    if (transitioning && millis() - transitionStartTime > transitionDuration + 5000) {
        Serial.println(F("PatternManager: Stuck transition detected, forcing completion"));
        completeTransition();
        isValid = false;
    }
    
    // Check for stuck pattern change flags
    if (patternChangeInProgress && millis() - lastPatternChange > 10000) {
        Serial.println(F("PatternManager: Stuck change flag detected, clearing"));
        patternChangeInProgress = false;
        isValid = false;
    }
    
    return isValid;
}

void PatternManager::printStatus() {
    Serial.println(F("=== PatternManager Status ==="));
    Serial.print(F("Current Pattern: "));
    Serial.print(currentPattern);
    Serial.print(F(" - "));
    Serial.println(patternNames[currentPattern]);
    
    Serial.print(F("Transitioning: "));
    Serial.println(transitioning ? "YES" : "NO");
    
    if (transitioning) {
        Serial.print(F("  Progress: "));
        Serial.print(transitionProgress);
        Serial.println(F("/255"));
        Serial.print(F("  From: "));
        Serial.print(fromPattern);
        Serial.print(F(" To: "));
        Serial.println(toPattern);
    }
    
    Serial.print(F("Change in Progress: "));
    Serial.println(patternChangeInProgress ? "YES" : "NO");
    
    Serial.print(F("Total Changes: "));
    Serial.println(patternChangeCount);
    
    Serial.print(F("Time Since Last Change: "));
    Serial.print(getTimeSinceLastChange());
    Serial.println(F("ms"));
    
    Serial.print(F("Transition Duration: "));
    Serial.print(transitionDuration);
    Serial.println(F("ms"));
    
    Serial.println(F("========================"));
}

unsigned long PatternManager::getTimeSinceLastChange() const {
    return millis() - lastPatternChange;
}

// Private methods

void PatternManager::startTransition(uint8_t newPattern, TransitionType type) {
    fromPattern = currentPattern;
    toPattern = newPattern;
    transitionType = type;
    transitioning = true;
    patternChangeInProgress = true;
    transitionStartTime = millis();
    transitionProgress = 0;
    
    Serial.print(F("PatternManager: Starting transition from "));
    Serial.print(fromPattern);
    Serial.print(F(" to "));
    Serial.print(toPattern);
    Serial.print(F(" (type: "));
    Serial.print((int)type);
    Serial.println(F(")"));
}

void PatternManager::updateTransition() {
    unsigned long elapsed = millis() - transitionStartTime;
    
    // Check for completion
    if (elapsed >= transitionDuration) {
        completeTransition();
        return;
    }
    
    // Calculate transition progress (0-255)
    transitionProgress = map(elapsed, 0, transitionDuration, 0, 255);
    
    // Apply transition effect
    applyTransitionEffect();
}

void PatternManager::completeTransition() {
    currentPattern = toPattern;
    transitioning = false;
    patternChangeInProgress = false;
    transitionProgress = 0;
    lastPatternChange = millis();
    patternChangeCount++;
    
    Serial.print(F("PatternManager: Transition complete - now on pattern "));
    Serial.print(currentPattern);
    Serial.print(F(" - "));
    Serial.println(patternNames[currentPattern]);
}

void PatternManager::applyTransitionEffect() {
    // This method can be expanded to implement actual transition effects
    // For now, patterns can check isTransitioning() and getTransitionProgress()
    // to implement their own transition behavior
    
    switch (transitionType) {
        case TRANSITION_FADE:
            // Patterns should gradually fade from old to new
            // Implementation depends on individual patterns
            break;
            
        case TRANSITION_WIPE:
            // Wipe effect across panels
            // Could be implemented here or in patterns
            break;
            
        case TRANSITION_FLASH:
            // Quick flash transition at midpoint
            if (transitionProgress >= 128 && currentPattern != toPattern) {
                currentPattern = toPattern;
                Serial.println(F("PatternManager: Flash transition - pattern switched"));
            }
            break;
            
        case TRANSITION_NONE:
        default:
            // Should not reach here, but handle gracefully
            currentPattern = toPattern;
            transitioning = false;
            patternChangeInProgress = false;
            Serial.println(F("PatternManager: No transition - immediate change"));
            break;
    }
}

bool PatternManager::validatePatternIndex(uint8_t pattern) const {
    return (pattern < NUM_PATTERNS);
}

bool PatternManager::canChangePattern() const {
    // Don't allow changes during critical operations
    if (patternChangeInProgress) {
        return false;
    }
    
    // Rate limiting - max 5 changes per second
    if (millis() - lastPatternChange < 200) {
        return false;
    }
    
    return true;
}

void PatternManager::logPatternChange(uint8_t fromPattern, uint8_t toPattern, const char* reason) {
    Serial.print(F("PatternManager: Changed from "));
    Serial.print(fromPattern);
    Serial.print(F(" ("));
    Serial.print(patternNames[fromPattern]);
    Serial.print(F(") to "));
    Serial.print(toPattern);
    Serial.print(F(" ("));
    Serial.print(patternNames[toPattern]);
    Serial.print(F(")"));
    
    if (reason) {
        Serial.print(F(" - "));
        Serial.print(reason);
    }
    
    Serial.print(F(" [Change #"));
    Serial.print(patternChangeCount + 1);
    Serial.println(F("]"));
}

bool PatternManager::acquireLEDMutex(uint32_t timeoutMs) const {
    if (ledMutex == nullptr) {
        Serial.println(F("PatternManager: LED mutex is null!"));
        return false;
    }
    
    return (xSemaphoreTake(ledMutex, pdMS_TO_TICKS(timeoutMs)) == pdTRUE);
}

void PatternManager::releaseLEDMutex() const {
    if (ledMutex != nullptr) {
        xSemaphoreGive(ledMutex);
    }
}