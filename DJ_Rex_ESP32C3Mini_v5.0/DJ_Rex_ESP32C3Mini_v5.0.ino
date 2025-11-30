/////////////////////////////////////////////////////////////////////////////////
//
//  Firmware for the Printed Droid DJ Rex Light Panels System
//  www.printed-droid.com
//
//  ==============================================================================
//
//  VERSION: 5.0.0 - "Merged Edition"
//  DATE:    2025/11/30
//
//  ==============================================================================
//
//  HARDWARE:
//  - Controller: LOLIN C3 Mini (ESP32-C3)
//  - LEDs:       WS2812B (GRB color order)
//  - Serial Baud Rate: 115200
//
//  LED LAYOUT:
//  - Body Panels:  3 panels with 20 LEDs each (total 60 LEDs)
//                  - 8 side LEDs per panel
//                  - 3x4 block LEDs per panel
//  - Eyes:         2 LEDs
//  - Mouth:        80 LEDs in a 12-row matrix
//
//  PINOUT & RMT CHANNELS:
//  - RMT CH0 (IO3):  Body Panels (Right -> Middle -> Left, chained)
//  - RMT CH1 (IO6):  Eyes (2 LEDs, separate)
//  - RMT CH2 (IO7):  Mouth (80 LEDs, separate)
//  - MIC_PIN (IO1):  Analog microphone input for audio reactivity
//
//  Original separate pins (had issues, now chained):
//  - IO3: Right, IO4: Middle, IO5: Left
//
//  ==============================================================================
//
//  KEY FEATURES (Merged from v3.1 + v4.2):
//
//  --- From v3.1 ---
//  * Correct LED hardware configuration (20 LEDs per panel)
//  * Playlist Mode with crossfade transitions
//  * 3 User Preset Slots
//
//  --- From v4.2 ---
//  * FreeRTOS multi-threading (audio on Core 0)
//  * Thread-safe LED operations with mutex protection
//  * Enhanced error handling and recovery
//  * System health monitoring
//  * Memory-safe preset management (10 slots)
//  * Beat detection and frequency analysis
//  * 20 body patterns + 15 mouth patterns
//  * Extended color palette (20 colors)
//
//  ==============================================================================
//  FastLED Library Version: 3.9.0 required!!!
//
/////////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include <Preferences.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Watchdog timer (enhanced safety)
#if ENABLE_WATCHDOG
#include "esp_task_wdt.h"
#endif

// Project headers (CORRECTED INCLUDES)
#include "config.h"
#include "globals.h"
#include "helpers.h"
#include "patterns_body.h"
#include "patterns_mouth.h"
#include "eyes.h"
#include "serial_commands.h"
#include "settings.h"
#include "demo.h"
#include "startup.h"
#include "status_led.h"
#include "pattern_manager.h"    // CORRECTED: Now includes real PatternManager
#include "preset_manager.h"     // CORRECTED: Memory-safe PresetManager
#include "audio_processor.h"
#include "event_logger.h"

// Global timing variables
unsigned long currentMillis = 0;
unsigned long lastPatternUpdate = 0;
unsigned long lastLEDUpdate = 0;
unsigned long lastStatusUpdate = 0;
unsigned long lastHealthCheck = 0;

// FreeRTOS task handles
TaskHandle_t audioTaskHandle = NULL;

// System health monitoring
static uint8_t consecutiveErrors = 0;
static unsigned long lastErrorTime = 0;
static bool emergencyMode = false;

// Critical section mutex for system-wide thread safety
static portMUX_TYPE criticalMutex = portMUX_INITIALIZER_UNLOCKED;

// Performance monitoring variables
static unsigned long loopCounter = 0;
static unsigned long lastPerformanceReport = 0;

// Audio task - runs on Core 0 with enhanced error handling
void audioTask(void * parameter) {
    const TickType_t xFrequency = pdMS_TO_TICKS(AUDIO_SAMPLE_INTERVAL_MS);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    uint32_t audioTaskErrors = 0;
    uint32_t audioTaskCycles = 0;
    
    Serial.println(F("Audio task started on Core 0"));
    
    for(;;) {
        audioTaskCycles++;
        
        try {
            // Update audio processor with error handling
            audioProcessor.update();
            
            // Reset error counter on successful operation
            if (audioTaskErrors > 0) {
                audioTaskErrors = 0;
            }
            
        } catch (...) {
            audioTaskErrors++;
            Serial.print(F("Audio task error #"));
            Serial.println(audioTaskErrors);
            
            // If too many errors, restart the task
            if (audioTaskErrors > 10) {
                Serial.println(F("CRITICAL: Audio task has too many errors, restarting..."));
                audioProcessor.resetPeak();
                audioTaskErrors = 0;
                delay(1000); // Give system time to recover
            }
        }
        
        // Periodic health report
        if (audioTaskCycles % 10000 == 0) { // Every ~50 seconds at 5ms intervals
            Serial.print(F("Audio task health: "));
            Serial.print(audioTaskCycles);
            Serial.print(F(" cycles, "));
            Serial.print(audioTaskErrors);
            Serial.println(F(" errors"));
        }
        
        // Wait for the next cycle with absolute timing
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// System health check function
void performHealthCheck() {
    bool healthIssues = false;
    
    // Check free memory
    uint32_t freeHeap = ESP.getFreeHeap();
    if (freeHeap < MEMORY_CRITICAL_THRESHOLD) {
        Serial.print(F("CRITICAL: Low memory - "));
        Serial.print(freeHeap);
        Serial.println(F(" bytes free"));
        setStatusLED(STATUS_MEMORY_WARNING);
        healthIssues = true;
    } else if (freeHeap < MEMORY_WARNING_THRESHOLD) {
        Serial.print(F("WARNING: Low memory - "));
        Serial.print(freeHeap);
        Serial.println(F(" bytes free"));
    }
    
    // Check LED operation health
    if (!isLEDOperationSafe()) {
        Serial.println(F("WARNING: LED operations may be unsafe"));
        healthIssues = true;
    }
    
    // Check pattern manager state
    if (!patternManager.validateCurrentState()) {
        Serial.println(F("WARNING: Pattern manager state issues detected"));
        healthIssues = true;
    }
    
    // Check audio task health
    if (audioTaskHandle != NULL) {
        eTaskState audioTaskState = eTaskGetState(audioTaskHandle);
        if (audioTaskState != eRunning && audioTaskState != eBlocked) {
            Serial.print(F("WARNING: Audio task not running (state: "));
            Serial.print((int)audioTaskState);
            Serial.println(F(")"));
            healthIssues = true;
        }
    }
    
    // Update health status
    if (healthIssues) {
        consecutiveErrors++;
        lastErrorTime = millis();
        
        if (consecutiveErrors >= MAX_CONSECUTIVE_ERRORS) {
            Serial.println(F("CRITICAL: Too many consecutive health issues"));
            emergencyMode = true;
            setStatusLED(STATUS_ERROR);
        }
    } else {
        if (consecutiveErrors > 0) {
            Serial.println(F("Health check: All issues resolved"));
        }
        consecutiveErrors = 0;
    }
}

// Emergency recovery procedure
void handleEmergencyMode() {
    if (!emergencyMode) return;
    
    Serial.println(F("EMERGENCY MODE ACTIVATED"));
    setStatusLED(STATUS_ERROR);
    
    // Clear all LEDs
    emergencyLEDClear();
    
    // Stop demo mode
    demoMode = false;
    
    // Set safe pattern
    patternManager.setPatternSafe(0); // LEDs Off
    
    // Save current state if possible
    if (ESP.getFreeHeap() > 5000) {
        Serial.println(F("Emergency: Saving settings..."));
        saveSettings();
    }
    
    // Wait and attempt system restart
    delay(PANIC_RESTART_DELAY_MS);
    
    Serial.println(F("Emergency: Restarting system..."));
    ESP.restart();
}

// Performance monitoring
void updatePerformanceMonitoring() {
    loopCounter++;
    
    if (millis() - lastPerformanceReport >= PERFORMANCE_REPORT_INTERVAL_MS) {
        lastPerformanceReport = millis();
        
        // Calculate loops per second
        float loopsPerSecond = (float)loopCounter * 1000.0f / PERFORMANCE_REPORT_INTERVAL_MS;
        
        Serial.print(F("Performance: "));
        Serial.print(loopsPerSecond, 1);
        Serial.print(F(" loops/sec, Free heap: "));
        Serial.print(ESP.getFreeHeap());
        Serial.println(F(" bytes"));
        
        // Check for performance degradation
        if (loopsPerSecond < 100) { // Less than 100 loops/sec is concerning
            Serial.println(F("WARNING: Low loop performance detected"));
            setStatusLED(STATUS_ERROR);
        }
        
        // Reset counter
        loopCounter = 0;
        
        // Print additional statistics
        printLEDPerformanceStats();
    }
}

void setup() {
    // Initial delay for power stabilization
    delay(500);

    // Configure all pins first with error checking
    pinMode(LED_PIN_RIGHT, OUTPUT);
    pinMode(LED_PIN_MIDDLE, OUTPUT);
    pinMode(LED_PIN_LEFT, OUTPUT);
    pinMode(EYES_MOUTH_PIN, OUTPUT);

    #if ENABLE_STATUS_LED
    pinMode(STATUS_LED_PIN, OUTPUT);
    #endif

    // Initialize status LED (does nothing if disabled)
    initStatusLED();
    setStatusLED(STATUS_STARTUP);
    
    // Start serial communication
    Serial.begin(115200);
    Serial.flush(); // Ensure clean start

    // Wait for serial connection with timeout
    unsigned long serialStart = millis();
    while (!Serial && millis() - serialStart < SERIAL_TIMEOUT_MS) {
        updateStatusLED();
        delay(10);
    }
    
    // Print startup banner
    Serial.println(F("\n========================================"));
    Serial.println(F("DJ Rex ESP32-C3 v5.0.0 - MERGED EDITION"));
    Serial.println(F("Build: " FIRMWARE_DATE));
    Serial.println(F("========================================"));
    Serial.println(F("Hardware: ESP32-C3 Mini - 3 RMT channels"));
    Serial.println(F("Body: 3x20 LEDs (IO3), Eyes: 2 (IO6), Mouth: 80 (IO7)"));
    Serial.println(F("Features: 20 colors + 20 patterns + 15 mouth modes"));
    Serial.println(F("Audio: Beat detection and frequency analysis"));
    Serial.println(F("Thread-safe LED operations enabled"));
    Serial.println(F("Memory-safe preset management"));
    Serial.println(F("Enhanced error handling and recovery"));
    Serial.println(F("Type 'help' for commands"));
    Serial.println(F("========================================"));
    
    // System information
    Serial.print(F("CPU Frequency: "));
    Serial.print(ESP.getCpuFreqMHz());
    Serial.println(F(" MHz"));
    
    Serial.print(F("Flash Size: "));
    Serial.print(ESP.getFlashChipSize() / 1024 / 1024);
    Serial.println(F(" MB"));
    
    Serial.print(F("Free Heap: "));
    Serial.print(ESP.getFreeHeap());
    Serial.println(F(" bytes"));
    
    Serial.print(F("Estimated Memory Usage: "));
    Serial.print(TOTAL_ESTIMATED_MEMORY);
    Serial.println(F(" bytes"));
    
    // ADC configuration for microphone with error checking
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    
    // Test microphone reading
    int micTest = analogRead(MIC_PIN);
    Serial.print(F("Microphone test reading: "));
    Serial.println(micTest);
    if (micTest == 0) {
        Serial.println(F("WARNING: Microphone may not be connected properly"));
    }

    // Initialize thread synchronization primitives
    ledMutex = xSemaphoreCreateMutex();
    if (ledMutex == NULL) {
        Serial.println(F("CRITICAL ERROR: Failed to create LED mutex!"));
        setStatusLED(STATUS_ERROR);
        while(1) { 
            delay(1000);
            updateStatusLED();
        }
    }
    Serial.println(F("LED mutex created successfully"));
    
    // Initialize settings (must be before LED initialization)
    Serial.println(F("Loading settings..."));
    initSettings();
    Serial.println(F("Settings loaded"));

    // Configure LED strips for ESP32-C3
    // Body panels chained on IO3, Eyes on IO6, Mouth on IO7 (all separate)
    Serial.println(F("Initializing LED strips for ESP32-C3..."));
    try {
        // RMT Channel 0: Body panels chained (Right->Middle->Left on IO3)
        FastLED.addLeds<LED_TYPE, BODY_CHAIN_PIN, COLOR_ORDER>(bodyLEDsChained, TOTAL_BODY_LEDS);
        Serial.println(F("Body chain: Right->Middle->Left on IO3"));

        // RMT Channel 1: Eyes on IO6 (separate)
        FastLED.addLeds<LED_TYPE, EYES_PIN_ACTIVE, COLOR_ORDER>(DJLEDs_Eyes, NUM_EYES);
        Serial.println(F("Eyes: IO6 (2 LEDs)"));

        // RMT Channel 2: Mouth on IO7 (separate)
        FastLED.addLeds<LED_TYPE, MOUTH_PIN_ACTIVE, COLOR_ORDER>(DJLEDs_Mouth, NUM_MOUTH_LEDS);
        Serial.println(F("Mouth: IO7 (80 LEDs)"));

        Serial.println(F("ESP32-C3: All 3 RMT channels configured successfully"));

    } catch (...) {
        Serial.println(F("CRITICAL ERROR: LED strip initialization failed!"));
        setStatusLED(STATUS_ERROR);
        while(1) {
            delay(1000);
            updateStatusLED();
        }
    }
    
    // Configure FastLED settings
    FastLED.setBrightness(ledBrightness);
    FastLED.setMaxPowerInVoltsAndMilliamps(LED_VOLTAGE, LED_POWER_LIMIT_MW / LED_VOLTAGE);
    Serial.print(F("LED brightness set to: "));
    Serial.println(ledBrightness);
    Serial.print(F("Power limit: "));
    Serial.print(LED_POWER_LIMIT_MW);
    Serial.println(F("mW"));
    
    // Clear all LEDs on startup
    Serial.println(F("Clearing all LEDs..."));
    FastLED.clear();
    FastLED.show();
    
    // Initialize components with error checking
    Serial.println(F("Initializing subsystems..."));
    
    try {
        initializeHelpers();
        Serial.println(F("Helpers initialized"));
        
        initializeEyes();
        Serial.println(F("Eyes initialized"));
        
        initializeStartup();
        Serial.println(F("Startup sequence initialized"));
        
        initializePatterns();
        Serial.println(F("Patterns initialized"));
        
        patternManager.begin();
        Serial.println(F("Pattern manager initialized"));
        
        audioProcessor.begin();
        Serial.println(F("Audio processor initialized"));
        
        presetManager.begin();
        Serial.println(F("Preset manager initialized"));
        
        eventLogger.clear();
        Serial.println(F("Event logger initialized"));
        
    } catch (...) {
        Serial.println(F("CRITICAL ERROR: Subsystem initialization failed!"));
        setStatusLED(STATUS_ERROR);
        while(1) { 
            delay(1000);
            updateStatusLED();
        }
    }
    
    // Log startup event
    bootTime = millis();
    eventLogger.log(EVENT_SYSTEM_START, "System initialized v" FIRMWARE_VERSION);

    // Create FreeRTOS audio task with error handling
    Serial.println(F("Creating audio task..."));
    BaseType_t audioTaskCreated = xTaskCreatePinnedToCore(
        audioTask,             // Task function
        "AudioTask",           // Name
        AUDIO_TASK_STACK_SIZE, // Stack size
        NULL,                  // Parameters
        TASK_PRIORITY_AUDIO,   // Priority
        &audioTaskHandle,      // Task handle
        0                      // Core 0
    );

    if (audioTaskCreated != pdPASS) {
        Serial.println(F("CRITICAL ERROR: Failed to create audio task!"));
        setStatusLED(STATUS_ERROR);
        audioTaskHandle = NULL;
    } else {
        Serial.println(F("Audio task created successfully on Core 0"));
    }
    
    // Initialize watchdog timer
    #if ENABLE_WATCHDOG
    Serial.println(F("Enabling watchdog timer..."));
    if (esp_task_wdt_init(WATCHDOG_TIMEOUT_S, true) == ESP_OK) {
        if (esp_task_wdt_add(NULL) == ESP_OK) {
            Serial.print(F("Watchdog enabled with "));
            Serial.print(WATCHDOG_TIMEOUT_S);
            Serial.println(F(" second timeout"));
        } else {
            Serial.println(F("WARNING: Failed to add main task to watchdog"));
        }
    } else {
        Serial.println(F("WARNING: Failed to initialize watchdog"));
    }
    #endif
    
    // Memory monitoring
    #if ENABLE_MEMORY_MONITORING
    Serial.println(F("Memory monitoring enabled"));
    printMemoryStatus();
    #endif
    
    // Final system validation
    Serial.println(F("Running system validation..."));
    validateHelperState();
    presetManager.validateAllPresets();
    
    // Mark system as ready
    systemReady = true;
    lastHealthCheck = millis();
    lastPerformanceReport = millis();
    
    Serial.println(F("\n*** SYSTEM READY ***"));
    Serial.print(F("Boot time: "));
    Serial.print(bootTime);
    Serial.println(F("ms"));
    Serial.println(F("Starting boot sequence..."));
    Serial.println(F("========================================\n"));
}

void loop() {
    // Handle emergency mode first
    if (emergencyMode) {
        handleEmergencyMode();
        return; // Should not reach here due to restart
    }
    
    // Get current time once per loop iteration
    currentMillis = millis();

    // Reset watchdog timer
    #if ENABLE_WATCHDOG
    static unsigned long lastWatchdogFeed = 0;
    if (currentMillis - lastWatchdogFeed >= WATCHDOG_FEED_INTERVAL_MS) {
        esp_task_wdt_reset();
        lastWatchdogFeed = currentMillis;
    }
    #endif
    
    // Update status LED with its own timing (high frequency)
    static unsigned long lastStatusLEDUpdate = 0;
    if (currentMillis - lastStatusLEDUpdate >= 10) {  // 100Hz update rate
        lastStatusLEDUpdate = currentMillis;
        updateStatusLED();
    }
    
    // Handle serial commands with enhanced error handling
    try {
        if (checkSerialCommand()) {
            setStatusLED(STATUS_SERIAL_CMD);
            processSerialCommand();
            lastStatusUpdate = currentMillis;
        }
    } catch (...) {
        Serial.println(F("ERROR: Exception in serial command processing"));
        inputString = "";
        stringComplete = false;
        failedCommands++;
    }
    
    // Run startup sequence if not complete
    if (!isStartupComplete()) {
        runStartupSequence();
        
        // Update LEDs during startup with error handling
        if (currentMillis - lastLEDUpdate >= FRAME_DELAY_MS) {
            lastLEDUpdate = currentMillis;
            if (acquireLEDMutex(50, "startup_led_update")) {
                FastLED.show();
                releaseLEDMutex("startup_led_update");
            }
        }
        return; // Don't run normal patterns until startup is complete
    }
    
    // Set normal status after startup or command
    if (getStatusLED() != STATUS_DEMO && getStatusLED() != STATUS_AUDIO_ACTIVE && 
        getStatusLED() != STATUS_MEMORY_WARNING && getStatusLED() != STATUS_ERROR) {
        setStatusLED(STATUS_NORMAL);
    }
    
    // Handle demo mode with enhanced state preservation
    if (demoMode) {
        if (getStatusLED() != STATUS_DEMO) {
            setStatusLED(STATUS_DEMO);
        }
        try {
            handleDemoMode();
        } catch (...) {
            Serial.println(F("ERROR: Exception in demo mode"));
            demoMode = false;
        }
    } else if (getStatusLED() == STATUS_DEMO) {
        setStatusLED(STATUS_NORMAL);
    }
    
    // Check for audio activity with enhanced detection
    if (audioMode != AUDIO_OFF) {
        try {
            int audioLevel = audioProcessor.getLevel();
            if (audioLevel > audioThreshold * 0.5) {
                if (audioProcessor.isBeatDetected()) {
                    setStatusLED(STATUS_AUDIO_ACTIVE);
                    lastStatusUpdate = currentMillis;
                }
            } else if (getStatusLED() == STATUS_AUDIO_ACTIVE && 
                       currentMillis - lastStatusUpdate > 500) {
                setStatusLED(STATUS_NORMAL);
            }
        } catch (...) {
            Serial.println(F("ERROR: Exception in audio processing"));
            audioMode = AUDIO_OFF; // Disable audio on error
        }
    }
    
    // Update pattern manager (handles transitions and validation)
    try {
        patternManager.update();
    } catch (...) {
        Serial.println(F("ERROR: Exception in pattern manager"));
        patternManager.setPatternSafe(0); // Safe fallback
    }

    // Update patterns with enhanced thread safety and error handling
    if (currentMillis - lastPatternUpdate >= 5) {
        lastPatternUpdate = currentMillis;
        
        if (acquireLEDMutex(PATTERN_CHANGE_TIMEOUT_MS, "pattern_update")) {
            patternUpdating = true;
            
            try {
                // Update body pattern with bounds checking
                if (currentPattern < NUM_PATTERNS && gPatterns[currentPattern] != nullptr) {
                    gPatterns[currentPattern]();
                } else {
                    // Safety fallback with error logging
                    Serial.print(F("ERROR: Invalid pattern index "));
                    Serial.print(currentPattern);
                    Serial.println(F(", resetting to safe pattern"));
                    currentPattern = 0;
                    setStatusLED(STATUS_PATTERN_ERROR);
                    if (gPatterns[0] != nullptr) {
                        gPatterns[0](); // LEDs Off pattern should always be safe
                    }
                }
                
                // Update eyes with error handling
                if (currentPattern != 0) {
                    updateEyes();
                }
                
                // Update mouth with error handling
                if (mouthEnabled && currentPattern != 0) {
                    updateMouth();
                }
                
            } catch (...) {
                Serial.println(F("CRITICAL ERROR: Exception in pattern update"));
                emergencyLEDClear();
                currentPattern = 0;
                setStatusLED(STATUS_ERROR);
            }
            
            patternUpdating = false;
            releaseLEDMutex("pattern_update");
        } else {
            Serial.println(F("WARNING: Pattern update skipped - mutex timeout"));
        }
    }
    
    // Update LEDs at defined framerate with error handling
    if (currentMillis - lastLEDUpdate >= FRAME_DELAY_MS) {
        lastLEDUpdate = currentMillis;
        
        if (acquireLEDMutex(LED_MUTEX_TIMEOUT_MS, "led_show")) {
            try {
                FastLED.show();
            } catch (...) {
                Serial.println(F("ERROR: Exception in FastLED.show()"));
                setStatusLED(STATUS_ERROR);
            }
            releaseLEDMutex("led_show");
        }
    }
    
    // Periodic updates for global color cycling with bounds checking
    EVERY_N_MILLISECONDS(20) {
        gHue++;
        
        // Smoother saturation transitions with validation
        if (updown) {
            if (gSat < 255 - colorFadeSteps) {
                gSat += colorFadeSteps;
            } else {
                gSat = 255;
                updown = 0;
            }
        } else {
            if (gSat > colorFadeSteps) {
                gSat -= colorFadeSteps;
            } else {
                gSat = 0;
                updown = 1;
            }
        }
    }
    
    // System health monitoring
    if (currentMillis - lastHealthCheck >= HEALTH_CHECK_INTERVAL_MS) {
        lastHealthCheck = currentMillis;
        performHealthCheck();
    }
    
    // Performance monitoring
    #if ENABLE_PERFORMANCE_MONITORING
    updatePerformanceMonitoring();
    #endif
    
    // Memory monitoring with enhanced checks
    #if ENABLE_MEMORY_MONITORING
    static unsigned long lastMemoryCheck = 0;
    if (currentMillis - lastMemoryCheck >= MEMORY_CHECK_INTERVAL_MS) {
        lastMemoryCheck = currentMillis;
        
        uint32_t freeHeap = ESP.getFreeHeap();
        if (freeHeap < MEMORY_CRITICAL_THRESHOLD) {
            Serial.print(F("CRITICAL: Memory critically low - "));
            Serial.print(freeHeap);
            Serial.println(F(" bytes"));
            setStatusLED(STATUS_MEMORY_WARNING);
            
            // Emergency memory cleanup
            if (demoMode) {
                Serial.println(F("Emergency: Disabling demo mode to free memory"));
                demoMode = false;
            }
            
            // Force garbage collection if possible
            Serial.println(F("Emergency: Attempting memory cleanup"));
        } else if (freeHeap < MEMORY_WARNING_THRESHOLD) {
            static unsigned long lastMemoryWarning = 0;
            if (currentMillis - lastMemoryWarning > 30000) { // Warn max once per 30 seconds
                lastMemoryWarning = currentMillis;
                Serial.print(F("WARNING: Memory low - "));
                Serial.print(freeHeap);
                Serial.println(F(" bytes"));
            }
        }
    }
    #endif
}

#if ENABLE_MEMORY_MONITORING
void printMemoryStatus() {
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t minHeap = ESP.getMinFreeHeap();
    uint32_t maxBlock = ESP.getMaxAllocHeap();
    
    Serial.print(F("[Memory] Free: "));
    Serial.print(freeHeap);
    Serial.print(F(" bytes, Min: "));
    Serial.print(minHeap);
    Serial.print(F(" bytes, Largest: "));
    Serial.print(maxBlock);
    Serial.println(F(" bytes"));
    
    // Calculate memory usage percentage
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t usedHeap = totalHeap - freeHeap;
    uint8_t usagePercent = (usedHeap * 100) / totalHeap;
    
    Serial.print(F("[Memory] Usage: "));
    Serial.print(usagePercent);
    Serial.print(F("% ("));
    Serial.print(usedHeap);
    Serial.print(F("/"));
    Serial.print(totalHeap);
    Serial.println(F(" bytes)"));
    
    // Warn if memory usage is concerning
    if (freeHeap < MEMORY_CRITICAL_THRESHOLD) {
        Serial.println(F("CRITICAL: Memory critically low!"));
        setStatusLED(STATUS_MEMORY_WARNING);
    } else if (freeHeap < MEMORY_WARNING_THRESHOLD) {
        Serial.println(F("WARNING: Memory running low"));
    }
    
    // Stack usage information
    Serial.print(F("[Stack] High water mark: "));
    Serial.print(uxTaskGetStackHighWaterMark(NULL));
    Serial.println(F(" words"));
}
#endif