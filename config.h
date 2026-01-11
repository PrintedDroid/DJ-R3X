#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <FastLED.h>

// =============================================================================
// VERSION INFO
// =============================================================================
#define FIRMWARE_VERSION "5.0.0"
#define FIRMWARE_DATE "2025-11-30"

// =============================================================================
// BOARD DETECTION
// =============================================================================
// Automatically detect ESP32 board type and configure accordingly
#if defined(CONFIG_IDF_TARGET_ESP32C3)
    #define BOARD_TYPE "ESP32-C3 Mini"
    #define IS_SINGLE_CORE true
    #define IS_DUAL_CORE false
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
    #define BOARD_TYPE "ESP32-S3 Mini"
    #define IS_SINGLE_CORE false
    #define IS_DUAL_CORE true
#else
    #warning "Unknown ESP32 board - defaulting to ESP32-C3 configuration"
    #define BOARD_TYPE "ESP32-C3 Mini (default)"
    #define IS_SINGLE_CORE true
    #define IS_DUAL_CORE false
#endif

// =============================================================================
// HARDWARE CONFIGURATION (from v3.1)
// =============================================================================
#define NUM_LEDS_PER_PANEL 20
#define NUM_EYES 2
#define NUM_MOUTH_LEDS 80
#define TOTAL_BODY_LEDS 60

// =============================================================================
// PIN DEFINITIONS - Board Specific
// =============================================================================
#if defined(CONFIG_IDF_TARGET_ESP32C3)
    // Pin definitions for ESP32-C3 Mini
    #define LED_PIN_RIGHT  3
    #define LED_PIN_MIDDLE 4
    #define LED_PIN_LEFT   5
    #define EYES_MOUTH_PIN 6
    #define MIC_PIN        1
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
    // Pin definitions for ESP32-S3 Mini
    #define LED_PIN_RIGHT  5
    #define LED_PIN_MIDDLE 6
    #define LED_PIN_LEFT   7
    #define EYES_MOUTH_PIN 8
    #define MIC_PIN        1
#else
    // Default to ESP32-C3 pins
    #define LED_PIN_RIGHT  3
    #define LED_PIN_MIDDLE 4
    #define LED_PIN_LEFT   5
    #define EYES_MOUTH_PIN 6
    #define MIC_PIN        1
#endif

// For compatibility
#define EYES_PIN EYES_MOUTH_PIN
#define MOUTH_PIN EYES_MOUTH_PIN

#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

// LED Layout definitions
#define SIDE_LEDS_START 0
#define SIDE_LEDS_COUNT 8
#define BLOCK1_START 8
#define BLOCK2_START 12
#define BLOCK3_START 16
#define LEDS_PER_BLOCK 4

// Mouth layout
#define MOUTH_ROWS 12
#define NUM_MOUTH_PATTERNS 15  // v5.0: Added Matrix, Heartbeat, Spectrum

// Color configuration
#define NUM_STANDARD_COLORS 20
#define RANDOM_COLOR_INDEX 19

// Timing
#define FRAMES_PER_SECOND 30
#define FRAME_DELAY_MS (1000 / FRAMES_PER_SECOND)
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
#define DECAYTIME 80

// =============================================================================
// v5.0 NEW: FREERTOS & THREAD SAFETY
// =============================================================================
// Automatically enabled on dual-core ESP32-S3, disabled on single-core ESP32-C3
// to prevent LED flickering on single-core chips
#if IS_DUAL_CORE
    #define ENABLE_FREERTOS_AUDIO true
    #define AUDIO_TASK_CORE 0  // Run audio task on Core 0, main loop on Core 1
#else
    #define ENABLE_FREERTOS_AUDIO false
#endif

#define AUDIO_TASK_STACK_SIZE 4096
#define AUDIO_TASK_PRIORITY 2
#define AUDIO_SAMPLE_INTERVAL_MS 5

// Mutex timeouts
#define LED_MUTEX_TIMEOUT_MS 100
#define PATTERN_CHANGE_TIMEOUT_MS 200

// =============================================================================
// v5.0 NEW: SYSTEM MONITORING
// =============================================================================
#define ENABLE_MEMORY_MONITORING true
#define MEMORY_WARNING_THRESHOLD 15000
#define MEMORY_CRITICAL_THRESHOLD 10000
#define MEMORY_CHECK_INTERVAL_MS 30000

#define ENABLE_HEALTH_CHECK true
#define HEALTH_CHECK_INTERVAL_MS 60000
#define MAX_CONSECUTIVE_ERRORS 5

// =============================================================================
// v5.0 NEW: PRESET MANAGER (10 slots instead of 3)
// =============================================================================
#define MAX_PRESETS 10
#define PRESET_NAME_LENGTH 16

// =============================================================================
// v5.0 NEW: STARTUP SEQUENCE
// =============================================================================
#define STARTUP_SEQUENCE_ENABLED true
#define STARTUP_PHASE_LED_TEST_MS 450
#define STARTUP_PHASE_EYES_MS 400
#define STARTUP_PHASE_MOUTH_MS 600
#define STARTUP_PHASE_SWEEP_MS 1500
#define STARTUP_PHASE_FLASH_MS 500

// =============================================================================
// v5.0 NEW: EVENT LOGGER
// =============================================================================
#define MAX_LOG_ENTRIES 20

// Event types
enum EventType {
    EVENT_SYSTEM_START,
    EVENT_PATTERN_CHANGE,
    EVENT_PRESET_LOAD,
    EVENT_PRESET_SAVE,
    EVENT_ERROR,
    EVENT_MEMORY_WARNING
};

// =============================================================================
// AUDIO MODES
// =============================================================================
enum AudioMode {
    AUDIO_OFF = 0,
    AUDIO_MOUTH_ONLY = 1,
    AUDIO_BODY_SIDES = 2,
    AUDIO_BODY_ALL = 3,
    AUDIO_ALL = 4
};

// Pattern count
#define NUM_PATTERNS 20  // v5.0: Added Plasma, Fire, Twinkle

#endif