#include "helpers.h"
#include <esp_system.h>
#include "status_led.h"

// Thread safety constants are now defined in config.h as macros
// LED_MUTEX_TIMEOUT_MS and CRITICAL_LED_TIMEOUT_MS

// Performance monitoring
static unsigned long ledOperationCount = 0;
static unsigned long ledMutexTimeouts = 0;
static unsigned long lastPerformanceReport = 0;

void initializeHelpers() {
    Serial.println(F("Helpers: Initializing..."));
    
    // Initialize random seed with hardware RNG
    uint32_t seed = esp_random();
    randomSeed(seed);
    Serial.print(F("Helpers: Random seed set to "));
    Serial.println(seed);
    
    // Initialize timing arrays with bounds checking and validation
    for (uint16_t x = 0; x < TOTAL_BODY_LEDS; x++) {
        IntervalTime[x] = random16(1000, 3000); // More reasonable range
        LEDMillis[x] = millis();
        LEDOn[x] = 0;
    }
    
    // Initialize matrix drops with proper bounds checking
    for (int panel = 0; panel < 3; panel++) {
        for (int led = 0; led < SIDE_LEDS_COUNT; led++) {
            matrixDrops[panel][led] = 255;
            matrixBright[panel][led] = 0;
        }
    }
    
    // Reset performance counters
    ledOperationCount = 0;
    ledMutexTimeouts = 0;
    lastPerformanceReport = millis();
    
    Serial.println(F("Helpers: Initialization complete"));
}

CRGB* getLEDArray(uint8_t panel) {
    // Strict bounds checking with error logging
    if (panel >= 3) {
        Serial.print(F("CRITICAL ERROR: Invalid panel index: "));
        Serial.println(panel);
        Serial.print(F("Stack trace: getLEDArray called from "));
        Serial.println((uintptr_t)__builtin_return_address(0), HEX);
        
        setStatusLED(STATUS_ERROR);
        return DJLEDs_Right; // Safe fallback to prevent crash
    }
    
    switch(panel) {
        case 0: return DJLEDs_Right;
        case 1: return DJLEDs_Middle;
        case 2: return DJLEDs_Left;
        default: 
            Serial.println(F("CRITICAL ERROR: Unreachable code in getLEDArray"));
            return DJLEDs_Right; // Should never reach here
    }
}

uint8_t getTimingIndex(uint8_t panel, uint8_t pos) {
    // Enhanced validation with recovery
    if (panel >= 3) {
        Serial.print(F("ERROR: Invalid panel in getTimingIndex: "));
        Serial.print(panel);
        Serial.print(F(" (pos: "));
        Serial.print(pos);
        Serial.println(F("), using panel 0"));
        panel = 0; // Safe fallback
    }
    
    if (pos >= NUM_LEDS_PER_PANEL) {
        Serial.print(F("ERROR: Invalid position in getTimingIndex: "));
        Serial.print(pos);
        Serial.print(F(" (panel: "));
        Serial.print(panel);
        Serial.println(F("), using position 0"));
        pos = 0; // Safe fallback
    }
    
    uint16_t index = panel * NUM_LEDS_PER_PANEL + pos;
    
    // Double-check calculated index
    if (index >= TOTAL_BODY_LEDS) {
        Serial.print(F("CRITICAL ERROR: Calculated index overflow: "));
        Serial.print(index);
        Serial.print(F(" >= "));
        Serial.println(TOTAL_BODY_LEDS);
        return 0; // Safe fallback
    }
    
    return index;
}

bool acquireLEDMutex(uint32_t timeoutMs, const char* caller) {
    if (ledMutex == nullptr) {
        Serial.print(F("CRITICAL ERROR: LED mutex is null in "));
        Serial.println(caller ? caller : "unknown function");
        setStatusLED(STATUS_ERROR);
        return false;
    }
    
    unsigned long startTime = millis();
    bool acquired = (xSemaphoreTake(ledMutex, pdMS_TO_TICKS(timeoutMs)) == pdTRUE);
    unsigned long waitTime = millis() - startTime;
    
    if (!acquired) {
        ledMutexTimeouts++;
        Serial.print(F("WARNING: LED mutex timeout in "));
        Serial.print(caller ? caller : "unknown function");
        Serial.print(F(" (waited "));
        Serial.print(waitTime);
        Serial.println(F("ms)"));
        
        // Log system state for debugging
        Serial.print(F("  Pattern updating: "));
        Serial.println(patternUpdating ? "YES" : "NO");
        Serial.print(F("  Current pattern: "));
        Serial.println(currentPattern);
        
        return false;
    }
    
    ledOperationCount++;
    
    // Periodic performance reporting
    if (millis() - lastPerformanceReport > 60000) { // Every minute
        printLEDPerformanceStats();
        lastPerformanceReport = millis();
    }
    
    return true;
}

void releaseLEDMutex(const char* caller) {
    if (ledMutex != nullptr) {
        xSemaphoreGive(ledMutex);
    } else {
        Serial.print(F("ERROR: Trying to release null LED mutex in "));
        Serial.println(caller ? caller : "unknown function");
    }
}

void setBlock(uint8_t panel, uint8_t blockStart, CRGB color) {
    // Comprehensive input validation
    if (panel >= 3) {
        Serial.print(F("ERROR: Invalid panel in setBlock: "));
        Serial.println(panel);
        return;
    }
    
    if (blockStart + LEDS_PER_BLOCK > NUM_LEDS_PER_PANEL) {
        Serial.print(F("ERROR: Block overflow in setBlock: panel="));
        Serial.print(panel);
        Serial.print(F(", start="));
        Serial.print(blockStart);
        Serial.print(F(", block_size="));
        Serial.print(LEDS_PER_BLOCK);
        Serial.print(F(", panel_size="));
        Serial.println(NUM_LEDS_PER_PANEL);
        return;
    }
    
    CRGB* leds = getLEDArray(panel);
    if (!leds) {
        Serial.println(F("CRITICAL ERROR: NULL LED array in setBlock"));
        setStatusLED(STATUS_ERROR);
        return;
    }
    
    CRGB adjustedColor = applyBodyBrightness(color);
    
    // Thread-safe LED operation with proper timeout
    if (acquireLEDMutex(LED_MUTEX_TIMEOUT_MS, "setBlock")) {
        // Verify we still have valid parameters after mutex acquisition
        if (panel < 3 && blockStart + LEDS_PER_BLOCK <= NUM_LEDS_PER_PANEL) {
            for (byte i = 0; i < LEDS_PER_BLOCK; i++) {
                leds[blockStart + i] = adjustedColor;
            }
        } else {
            Serial.println(F("ERROR: Parameters changed during setBlock mutex wait"));
        }
        releaseLEDMutex("setBlock");
    } else {
        Serial.print(F("WARNING: setBlock failed for panel "));
        Serial.print(panel);
        Serial.print(F(", block "));
        Serial.println(blockStart);
    }
}

void fadeBlock(uint8_t panel, uint8_t blockStart, uint8_t fadeAmount) {
    // Input validation
    if (panel >= 3) {
        Serial.print(F("ERROR: Invalid panel in fadeBlock: "));
        Serial.println(panel);
        return;
    }
    
    if (blockStart + LEDS_PER_BLOCK > NUM_LEDS_PER_PANEL) {
        Serial.print(F("ERROR: Block overflow in fadeBlock: panel="));
        Serial.print(panel);
        Serial.print(F(", start="));
        Serial.println(blockStart);
        return;
    }
    
    // Validate fade amount
    fadeAmount = constrain(fadeAmount, 0, 255);
    
    CRGB* leds = getLEDArray(panel);
    if (!leds) {
        Serial.println(F("CRITICAL ERROR: NULL LED array in fadeBlock"));
        return;
    }
    
    // Thread-safe fade operation
    if (acquireLEDMutex(LED_MUTEX_TIMEOUT_MS, "fadeBlock")) {
        for (byte i = 0; i < LEDS_PER_BLOCK; i++) {
            leds[blockStart + i].fadeToBlackBy(fadeAmount);
        }
        releaseLEDMutex("fadeBlock");
    } else {
        Serial.println(F("WARNING: fadeBlock failed - mutex timeout"));
    }
}

CRGB getColor(uint8_t colorIndex) {
    // Handle special cases first
    if (colorIndex == 10) { // Reserved for "Random"
        return CHSV(random8(), 255, 255);
    }
    
    // Bounds checking with fallback
    if (colorIndex >= NUM_STANDARD_COLORS) {
        Serial.print(F("WARNING: Invalid color index: "));
        Serial.print(colorIndex);
        Serial.println(F(", using Red"));
        return StandardColors[0]; // Fallback to Red
    }
    
    // Return standard color
    return StandardColors[colorIndex];
}

CRGB getSideLEDColor() {
    // Validate color mode with recovery
    if (sideColorMode > 4) {
        Serial.print(F("ERROR: Invalid side color mode: "));
        Serial.print(sideColorMode);
        Serial.println(F(", resetting to 0"));
        sideColorMode = 0;
    }
    
    // Validate individual color indices
    if (sideColor1 >= NUM_STANDARD_COLORS) {
        Serial.println(F("WARNING: Invalid sideColor1, resetting to 0"));
        sideColor1 = 0;
    }
    if (sideColor2 >= NUM_STANDARD_COLORS) {
        Serial.println(F("WARNING: Invalid sideColor2, resetting to 2"));
        sideColor2 = 2;
    }
    if (sideColor3 >= NUM_STANDARD_COLORS) {
        Serial.println(F("WARNING: Invalid sideColor3, resetting to 3"));
        sideColor3 = 3;
    }
    
    switch (sideColorMode) {
        case 0: // Random from 3
            {
                uint8_t colors[3] = {sideColor1, sideColor2, sideColor3};
                uint8_t selectedColor = colors[random(3)];
                return applyBodyBrightness(getColor(selectedColor));
            }
            
        case 1: // Cycle through 3
            {
                uint8_t colors[3] = {sideColor1, sideColor2, sideColor3};
                CRGB color = applyBodyBrightness(getColor(colors[sideColorCycleIndex]));
                sideColorCycleIndex = (sideColorCycleIndex + 1) % 3;
                return color;
            }
            
        case 2: return applyBodyBrightness(getColor(sideColor1));
        case 3: return applyBodyBrightness(getColor(sideColor2));
        case 4: return applyBodyBrightness(getColor(sideColor3));
        
        default:
            Serial.print(F("ERROR: Unhandled side color mode: "));
            Serial.println(sideColorMode);
            return applyBodyBrightness(getColor(sideColor1));
    }
}

CRGB getBlockColor(uint8_t blockIndex) {
    if (blockIndex >= 9) {
        Serial.print(F("WARNING: Block index overflow: "));
        Serial.print(blockIndex);
        Serial.println(F(", wrapping around"));
        blockIndex = blockIndex % 9; // Wrap around safely
    }
    
    // Validate the color index stored for this block
    uint8_t colorIndex = blockColors[blockIndex];
    if (colorIndex >= NUM_STANDARD_COLORS && colorIndex != 10) {
        Serial.print(F("WARNING: Invalid block color index: "));
        Serial.print(colorIndex);
        Serial.print(F(" for block "));
        Serial.print(blockIndex);
        Serial.println(F(", using default"));
        blockColors[blockIndex] = blockIndex % NUM_STANDARD_COLORS;
        colorIndex = blockColors[blockIndex];
    }
    
    return applyBodyBrightness(getColor(colorIndex));
}

uint8_t getGlobalBlockIndex(uint8_t panel, uint8_t localBlock) {
    // Strict input validation
    if (panel >= 3) {
        Serial.print(F("ERROR: Invalid panel in getGlobalBlockIndex: "));
        Serial.print(panel);
        Serial.println(F(", using panel 0"));
        panel = 0;
    }
    
    if (localBlock >= 3) {
        Serial.print(F("ERROR: Invalid local block in getGlobalBlockIndex: "));
        Serial.print(localBlock);
        Serial.println(F(", using block 0"));
        localBlock = 0;
    }
    
    // Calculate with explicit bounds checking
    // Panel mapping: Left=2, Middle=1, Right=0
    // Global index: Left blocks 0-2, Middle blocks 3-5, Right blocks 6-8
    uint8_t reversedPanel = 2 - panel;
    uint8_t result = reversedPanel * 3 + localBlock;
    
    // Validate result
    if (result >= 9) {
        Serial.print(F("CRITICAL ERROR: Block index calculation error: "));
        Serial.print(F("panel="));
        Serial.print(panel);
        Serial.print(F(", localBlock="));
        Serial.print(localBlock);
        Serial.print(F(", calculated="));
        Serial.println(result);
        return 0; // Safe fallback
    }
    
    return result;
}

uint16_t getRandomTiming(uint16_t minTime, uint16_t maxTime) {
    // Input validation and correction
    if (minTime >= maxTime) {
        Serial.print(F("WARNING: Invalid timing range: min="));
        Serial.print(minTime);
        Serial.print(F(", max="));
        Serial.print(maxTime);
        maxTime = minTime + 500; // Add 500ms minimum difference
        Serial.print(F(", corrected max="));
        Serial.println(maxTime);
    }
    
    // Validate effect speed
    if (effectSpeed == 0) {
        Serial.println(F("WARNING: Effect speed is 0, setting to 1"));
        effectSpeed = 1;
    }
    
    // Constrain inputs to reasonable ranges
    minTime = constrain(minTime, 50, 30000);   // 50ms to 30s
    maxTime = constrain(maxTime, 100, 60000);  // 100ms to 60s
    
    uint16_t baseTime = random(minTime, maxTime);
    
    // Apply speed scaling with overflow protection
    uint32_t adjustedTime = ((uint32_t)baseTime * 256UL) / effectSpeed;
    
    // Constrain final result to reasonable bounds
    uint16_t result = constrain(adjustedTime, 50, 30000);
    
    return result;
}

uint16_t getRandomTimingWithRate(uint16_t minTime, uint16_t maxTime, uint8_t rate) {
    // Input validation
    if (minTime >= maxTime) {
        maxTime = minTime + 500;
    }
    
    if (rate == 0) {
        Serial.println(F("WARNING: Rate is 0, setting to 1"));
        rate = 1;
    }
    
    // Constrain inputs
    minTime = constrain(minTime, 50, 30000);
    maxTime = constrain(maxTime, 100, 60000);
    rate = constrain(rate, 1, 255);
    
    uint16_t baseTime = random(minTime, maxTime);
    
    // Apply rate scaling with overflow protection
    uint32_t adjustedTime = ((uint32_t)baseTime * 256UL) / rate;
    
    return constrain(adjustedTime, 50, 30000);
}

CRGB applyBodyBrightness(CRGB color) {
    // Validate brightness percentage with recovery
    uint8_t brightness = bodyBrightness;
    if (brightness > 200) {
        Serial.print(F("WARNING: Body brightness too high: "));
        Serial.print(brightness);
        Serial.println(F("%, capping at 200%"));
        brightness = 200;
        bodyBrightness = 200; // Fix the global variable
    }
    
    if (brightness == 0) {
        Serial.println(F("WARNING: Body brightness is 0%, setting to 1%"));
        brightness = 1;
        bodyBrightness = 1;
    }
    
    // Apply brightness scaling with overflow protection
    CRGB adjustedColor;
    adjustedColor.r = min(255, ((uint16_t)color.r * brightness) / 100);
    adjustedColor.g = min(255, ((uint16_t)color.g * brightness) / 100);
    adjustedColor.b = min(255, ((uint16_t)color.b * brightness) / 100);
    
    return adjustedColor;
}

CRGB smoothColorTransition(CRGB currentColor, CRGB targetColor, uint8_t steps) {
    // Feature toggle check
    if (!enableSmoothTransitions) {
        return targetColor;
    }
    
    // Input validation
    if (steps == 0) {
        Serial.println(F("WARNING: Transition steps is 0, returning target color"));
        return targetColor;
    }
    
    if (steps > 64) {
        Serial.println(F("WARNING: Too many transition steps, capping at 64"));
        steps = 64;
    }
    
    // Calculate step increments with signed arithmetic
    int16_t rStep = ((int16_t)targetColor.r - (int16_t)currentColor.r) / steps;
    int16_t gStep = ((int16_t)targetColor.g - (int16_t)currentColor.g) / steps;
    int16_t bStep = ((int16_t)targetColor.b - (int16_t)currentColor.b) / steps;
    
    // Apply one transition step with bounds checking
    CRGB result;
    result.r = constrain((int16_t)currentColor.r + rStep, 0, 255);
    result.g = constrain((int16_t)currentColor.g + gStep, 0, 255);
    result.b = constrain((int16_t)currentColor.b + bStep, 0, 255);
    
    return result;
}

void validateHelperState() {
    Serial.println(F("=== Helper State Validation ==="));
    
    bool hasErrors = false;
    
    // Check global variables for corruption
    if (currentPattern >= NUM_PATTERNS) {
        Serial.print(F("ERROR: Invalid current pattern: "));
        Serial.println(currentPattern);
        hasErrors = true;
    }
    
    if (bodyBrightness > 200) {
        Serial.print(F("ERROR: Body brightness out of range: "));
        Serial.println(bodyBrightness);
        hasErrors = true;
    }
    
    if (effectSpeed == 0) {
        Serial.println(F("ERROR: Effect speed is zero"));
        hasErrors = true;
    }
    
    if (sideColorMode > 4) {
        Serial.print(F("ERROR: Invalid side color mode: "));
        Serial.println(sideColorMode);
        hasErrors = true;
    }
    
    // Check block colors
    for (int i = 0; i < 9; i++) {
        if (blockColors[i] >= NUM_STANDARD_COLORS && blockColors[i] != 10) {
            Serial.print(F("ERROR: Invalid block color at index "));
            Serial.print(i);
            Serial.print(F(": "));
            Serial.println(blockColors[i]);
            hasErrors = true;
        }
    }
    
    // Check timing arrays for reasonable values
    uint16_t suspiciousCount = 0;
    for (uint16_t i = 0; i < TOTAL_BODY_LEDS; i++) {
        if (IntervalTime[i] > 60000 || IntervalTime[i] < 10) { // Outside reasonable range
            suspiciousCount++;
        }
    }
    
    if (suspiciousCount > TOTAL_BODY_LEDS / 4) { // More than 25% suspicious
        Serial.print(F("WARNING: "));
        Serial.print(suspiciousCount);
        Serial.println(F(" timing values are suspicious"));
        hasErrors = true;
    }
    
    if (hasErrors) {
        Serial.println(F("Helper state validation FAILED"));
        setStatusLED(STATUS_ERROR);
    } else {
        Serial.println(F("Helper state validation PASSED"));
    }
    
    Serial.println(F("==============================="));
}

void printLEDPerformanceStats() {
    Serial.println(F("=== LED Performance Statistics ==="));
    Serial.print(F("Total LED operations: "));
    Serial.println(ledOperationCount);
    Serial.print(F("Mutex timeouts: "));
    Serial.println(ledMutexTimeouts);
    
    if (ledOperationCount > 0) {
        uint32_t timeoutRate = (ledMutexTimeouts * 100) / ledOperationCount;
        Serial.print(F("Timeout rate: "));
        Serial.print(timeoutRate);
        Serial.println(F("%"));
        
        if (timeoutRate > 5) { // More than 5% timeout rate is concerning
            Serial.println(F("WARNING: High mutex timeout rate detected!"));
            Serial.println(F("  This may indicate LED update performance issues"));
        }
    }
    
    Serial.print(F("Free heap: "));
    Serial.print(ESP.getFreeHeap());
    Serial.println(F(" bytes"));
    
    Serial.println(F("================================="));
}

void resetLEDPerformanceStats() {
    ledOperationCount = 0;
    ledMutexTimeouts = 0;
    lastPerformanceReport = millis();
    Serial.println(F("LED performance statistics reset"));
}

bool isLEDOperationSafe() {
    // Check if it's safe to perform LED operations
    if (ledMutex == nullptr) {
        Serial.println(F("LED operation unsafe: mutex is null"));
        return false;
    }
    
    if (patternUpdating) {
        return false; // Don't interfere with pattern updates
    }
    
    // Check for excessive timeout rate
    if (ledOperationCount > 100 && ledMutexTimeouts > 0) {
        uint32_t timeoutRate = (ledMutexTimeouts * 100) / ledOperationCount;
        if (timeoutRate > 20) { // More than 20% is critical
            Serial.println(F("LED operation unsafe: excessive timeout rate"));
            return false;
        }
    }
    
    return true;
}

void emergencyLEDClear() {
    Serial.println(F("EMERGENCY: Clearing all LEDs..."));
    
    // Try to acquire mutex with extended timeout for emergency
    if (acquireLEDMutex(CRITICAL_LED_TIMEOUT_MS, "emergencyLEDClear")) {
        // Clear all LED arrays
        fill_solid(DJLEDs_Right, NUM_LEDS_PER_PANEL, CRGB::Black);
        fill_solid(DJLEDs_Middle, NUM_LEDS_PER_PANEL, CRGB::Black);
        fill_solid(DJLEDs_Left, NUM_LEDS_PER_PANEL, CRGB::Black);
        fill_solid(DJLEDs_Eyes, NUM_EYES, CRGB::Black);
        fill_solid(DJLEDs_Mouth, NUM_MOUTH_LEDS, CRGB::Black);
        
        // Force LED update
        FastLED.show();
        
        releaseLEDMutex("emergencyLEDClear");
        Serial.println(F("Emergency LED clear complete"));
    } else {
        Serial.println(F("CRITICAL: Emergency LED clear failed - mutex timeout"));
        // In extreme emergency, could consider bypassing mutex here
        // but this would be dangerous and should be a last resort
    }
}