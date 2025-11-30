#ifndef CONFIG_H
#define CONFIG_H

// This line prevents FastLED from using its internal, outdated ArduinoJson library.
// It must come before the #include <FastLED.h>
#define FASTLED_NO_ARDUINOJSON

// #define FASTLED_ESP32_I2S true
#define FASTLED_INTERNAL

#include <FastLED.h>
#include <Arduino.h>

// Version information
#define FIRMWARE_VERSION "5.0.0"
#define FIRMWARE_DATE "2025-11"
#define BUILD_NUMBER 20251130

// Hardware configuration (CORRECTED from v3.1)
#define NUM_LEDS_PER_PANEL 20
#define NUM_EYES 2
#define NUM_MOUTH_LEDS 80
#define TOTAL_BODY_LEDS (NUM_LEDS_PER_PANEL * 3)  // = 60

// Pin definitions for ESP32-C3
// Original: separate pins per panel (had issues with individual control)
// Current: panels chained on single pin, eyes and mouth separate
const uint8_t BODY_CHAIN_PIN = 3;      // IO3 - Right->Middle->Left (chained)
const uint8_t EYES_PIN_ACTIVE = 6;     // IO6 - Eyes (separate)
const uint8_t MOUTH_PIN_ACTIVE = 7;    // IO7 - Mouth (separate)
const uint8_t MIC_PIN = 1;             // IO1 - Microphone

// Status LED - DISABLED (not present in hardware)
#define ENABLE_STATUS_LED false
const uint8_t STATUS_LED_PIN = 8;      // IO8 - Not used

// Legacy pin definitions (for code compatibility)
const uint8_t LED_PIN_RIGHT = 3;       // Original separate pin
const uint8_t LED_PIN_MIDDLE = 4;      // Original separate pin
const uint8_t LED_PIN_LEFT = 5;        // Original separate pin
const uint8_t EYES_MOUTH_PIN = 6;      // Legacy compatibility

// For compatibility with existing code
#define EYES_PIN EYES_PIN_ACTIVE
#define MOUTH_PIN MOUTH_PIN_ACTIVE

// LED hardware configuration
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

// LED Layout definitions (CORRECTED from v3.1)
#define SIDE_LEDS_START 0
#define SIDE_LEDS_COUNT 8
#define BLOCK1_START 8
#define BLOCK2_START 12
#define BLOCK3_START 16
#define LEDS_PER_BLOCK 4

// Mouth layout constants
#define MOUTH_ROWS 12

// Timing and performance
#define FRAMES_PER_SECOND 60      
#define FRAME_DELAY_MS (1000 / FRAMES_PER_SECOND)
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
#define DECAYTIME 80

// Audio configuration (enhanced)
#define AUDIO_SAMPLE_INTERVAL_MS 5   
#define BEAT_THRESHOLD 1.3f           
#define AUDIO_MIN_THRESHOLD 30        // Minimum safe threshold
#define AUDIO_MAX_THRESHOLD 800       // Maximum safe threshold
#define AUDIO_DEFAULT_THRESHOLD 100   // Safe default

// Serial command configuration (enhanced safety)
#define MAX_COMMAND_LENGTH 80         // Reduced from 100 for safety
#define SERIAL_TIMEOUT_MS 5000        // Increased timeout
#define SERIAL_BUFFER_SAFETY_MARGIN 10 // Extra safety margin

// Memory management constants
#define MEMORY_WARNING_THRESHOLD 15000   // Warn below 15KB free
#define MEMORY_CRITICAL_THRESHOLD 10000  // Critical below 10KB free
#define STACK_SAFETY_MARGIN 1024        // Reserve stack space

// Thread safety constants
#define LED_MUTEX_TIMEOUT_MS 200         // Standard LED operation timeout
#define CRITICAL_LED_TIMEOUT_MS 500      // Emergency/critical operation timeout
#define PATTERN_CHANGE_TIMEOUT_MS 300    // Pattern change timeout
#define MAX_MUTEX_WAIT_COUNT 5           // Max mutex wait attempts

// Performance monitoring
#define ENABLE_PERFORMANCE_MONITORING true
#define PERFORMANCE_REPORT_INTERVAL_MS 60000  // Report every minute
#define LED_TIMEOUT_RATE_WARNING 5            // Warn if >5% timeouts
#define LED_TIMEOUT_RATE_CRITICAL 20          // Critical if >20% timeouts

// Pattern-specific constants (validated ranges)
#define SHORT_CIRCUIT_SPARK_CHANCE 150      
#define GLITTER_CHANCE 80                   
#define JUGGLE_FADE_RATE 20                 
#define AUDIO_SYNC_FADE_RATE 20             
#define AUDIO_HIGH_LEVEL_THRESHOLD 0.8f      
#define AUDIO_MEDIUM_LEVEL_THRESHOLD 0.7f    
#define AUDIO_SPARKLE_CHANCE 50             
#define KNIGHT_RIDER_FADE 128               
#define MATRIX_DROP_CHANCE 50               
#define MATRIX_FADE_STEP 30                 
#define VU_METER_SPARKLE_THRESHOLD 0.9f      
#define VU_METER_SPARKLE_CHANCE 100         
#define CONFETTI_SPLIT_CHANCE 128           

// Startup sequence constants
#define STARTUP_DURATION_MS 3000            
#define STARTUP_FLICKER_CHANCE 200          

// Watchdog configuration (enhanced)
#define WATCHDOG_TIMEOUT_S 15               // Increased timeout
#define ENABLE_WATCHDOG false               // Disabled by default for stability
#define WATCHDOG_FEED_INTERVAL_MS 5000      // Feed every 5 seconds

// Storage and preset management
#define MAX_PRESETS 10                      
#define PRESET_NAME_LENGTH 32               
#define MAX_LOG_ENTRIES 50
#define PRESET_STORAGE_VERSION 1            // For future compatibility
#define ENABLE_PRESET_CHECKSUM true         // Enable checksum validation
#define ENABLE_PRESET_BACKUP false          // Disable backup for memory saving

// JSON export feature control - DEAKTIVIERT FÜR KOMPILIERUNG
#define ENABLE_JSON_EXPORT false            // Disabled to save memory
#define JSON_DOCUMENT_SIZE 1024             // Size when enabled

// Validation and safety features
#define ENABLE_INPUT_VALIDATION true        // Enable comprehensive validation
#define ENABLE_BOUNDS_CHECKING true         // Enable array bounds checking
#define ENABLE_THREAD_SAFETY_CHECKS true    // Enable mutex validation
#define ENABLE_MEMORY_MONITORING true       // Enable memory monitoring
#define MEMORY_CHECK_INTERVAL_MS 10000      // Check every 10 seconds

// Debug and development features
#define ENABLE_DEBUG_OUTPUT false           // Disable for production
#define ENABLE_VERBOSE_LOGGING false        // Disable verbose logs
#define ENABLE_STACK_MONITORING false       // Disable stack monitoring
#define ENABLE_HEAP_DEBUGGING false         // Disable heap debugging

// Status LED patterns (enhanced)
enum StatusLEDPattern {
    STATUS_OFF = 0,
    STATUS_STARTUP,
    STATUS_NORMAL,
    STATUS_ERROR,
    STATUS_DEMO,
    STATUS_AUDIO_ACTIVE,
    STATUS_SERIAL_CMD,
    STATUS_MEMORY_WARNING,
    STATUS_MUTEX_TIMEOUT,
    STATUS_PATTERN_ERROR
};

// Audio modes (validated)
enum AudioMode {
    AUDIO_OFF = 0,
    AUDIO_MOUTH_ONLY = 1,
    AUDIO_BODY_SIDES = 2,
    AUDIO_BODY_ALL = 3,
    AUDIO_ALL = 4,
    AUDIO_FREQ_BANDS = 5    
};

// Eye modes (validated)
enum EyeMode {
    EYE_MODE_SINGLE = 0,     
    EYE_MODE_DUAL = 1,       
    EYE_MODE_ALTERNATE = 2,  
    EYE_MODE_RAINBOW = 3,    
    EYE_MODE_AUDIO = 4       
};

// Mouth split modes (validated)
enum MouthSplitMode {
    MOUTH_SPLIT_OFF = 0,        
    MOUTH_SPLIT_LEFT_RIGHT = 1, 
    MOUTH_SPLIT_ALTERNATING = 2, 
    MOUTH_SPLIT_CENTER_OUT = 3,  
    MOUTH_SPLIT_TOP_BOTTOM = 4   
};

// Pattern and color counts (validated)
#define NUM_PATTERNS 20              
#define NUM_MOUTH_PATTERNS 15        
#define NUM_STANDARD_COLORS 20

// Event logging types (enhanced)
enum EventType {
    EVENT_SYSTEM_START,
    EVENT_PATTERN_CHANGE,
    EVENT_PRESET_LOAD,
    EVENT_PRESET_SAVE,
    EVENT_ERROR,
    EVENT_BEAT_DETECTED,
    EVENT_MEMORY_WARNING,
    EVENT_MUTEX_TIMEOUT,
    EVENT_VALIDATION_FAILED,
    EVENT_EMERGENCY_STOP
};

// FreeRTOS Configuration (enhanced)
#define AUDIO_TASK_STACK_SIZE 4096          // Adequate stack size
#define TASK_PRIORITY_AUDIO 2               // Medium priority
#define MAIN_TASK_STACK_SIZE 8192           // Large stack for main task
#define TASK_WATCHDOG_TIMEOUT_S 30          // Task watchdog timeout

// Error recovery configuration
#define ENABLE_AUTO_RECOVERY true           // Enable automatic error recovery
#define MAX_ERROR_COUNT 5                   // Max errors before emergency stop
#define ERROR_RECOVERY_DELAY_MS 1000        // Delay between recovery attempts
#define EMERGENCY_STOP_TIMEOUT_MS 5000      // Emergency stop timeout

// LED strip power management
#define LED_POWER_LIMIT_MW 2000             // 2A at 5V
#define LED_VOLTAGE 5                       // 5V power supply
#define ENABLE_POWER_MANAGEMENT true        // Enable power limiting

// Communication timeouts
#define I2C_TIMEOUT_MS 100                  // I2C operation timeout
#define SPI_TIMEOUT_MS 50                   // SPI operation timeout
#define UART_TIMEOUT_MS 1000                // UART timeout

// Validation macros for safety
#define VALIDATE_ARRAY_INDEX(idx, max) \
    do { \
        if ((idx) >= (max)) { \
            Serial.print(F("BOUNDS ERROR: ")); \
            Serial.print(#idx); \
            Serial.print(F(" = ")); \
            Serial.print(idx); \
            Serial.print(F(" >= ")); \
            Serial.println(max); \
            return; \
        } \
    } while(0)

#define VALIDATE_POINTER(ptr) \
    do { \
        if ((ptr) == nullptr) { \
            Serial.print(F("NULL POINTER ERROR: ")); \
            Serial.println(#ptr); \
            return; \
        } \
    } while(0)

#define VALIDATE_RANGE(val, min_val, max_val) \
    do { \
        if ((val) < (min_val) || (val) > (max_val)) { \
            Serial.print(F("RANGE ERROR: ")); \
            Serial.print(#val); \
            Serial.print(F(" = ")); \
            Serial.print(val); \
            Serial.print(F(" not in range [")); \
            Serial.print(min_val); \
            Serial.print(F(", ")); \
            Serial.print(max_val); \
            Serial.println(F("]")); \
            return; \
        } \
    } while(0)

// Memory alignment macros (for ESP32-C3 optimization)
#define ALIGN_4_BYTES __attribute__((aligned(4)))
#define ALIGN_8_BYTES __attribute__((aligned(8)))

// Compiler optimization hints
#define LIKELY(x)       __builtin_expect(!!(x), 1)
#define UNLIKELY(x)     __builtin_expect(!!(x), 0)

// FORCE_INLINE FIX - Nur definieren wenn nicht bereits definiert
#ifndef FORCE_INLINE
#define FORCE_INLINE    __attribute__((always_inline)) inline
#endif

// Critical section macros for thread safety
#define ENTER_CRITICAL_SECTION() portENTER_CRITICAL(&criticalMutex)
#define EXIT_CRITICAL_SECTION()  portEXIT_CRITICAL(&criticalMutex)

// Feature toggles for memory optimization
#define ENABLE_SMOOTH_TRANSITIONS true      // Smooth color transitions
#define ENABLE_ADVANCED_PATTERNS true       // Advanced pattern features
#define ENABLE_EXTENDED_COLORS true         // Extended color palette
#define ENABLE_PATTERN_TRANSITIONS false    // Pattern transition effects (memory intensive)
#define ENABLE_AUDIO_FFT false              // FFT analysis (memory intensive)

// System health monitoring
#define HEALTH_CHECK_INTERVAL_MS 30000      // Health check every 30 seconds
#define MAX_CONSECUTIVE_ERRORS 3            // Max errors before restart
#define SYSTEM_RESTART_DELAY_MS 5000        // Delay before system restart

// Panic and emergency configurations
#define ENABLE_PANIC_MODE true              // Enable emergency panic mode
#define PANIC_LED_CLEAR true                // Clear LEDs in panic
#define PANIC_SAVE_STATE true               // Save state before panic restart
#define PANIC_RESTART_DELAY_MS 3000         // Delay before panic restart

// Build configuration validation (CORRECTED for v3.1 hardware)
#if NUM_LEDS_PER_PANEL != 20
    #error "NUM_LEDS_PER_PANEL must be 20 for this hardware configuration"
#endif

#if TOTAL_BODY_LEDS != 60
    #error "TOTAL_BODY_LEDS calculation is incorrect (should be 60)"
#endif

#if MAX_COMMAND_LENGTH > 100
    #error "MAX_COMMAND_LENGTH too large - buffer overflow risk"
#endif

#if AUDIO_SAMPLE_INTERVAL_MS < 1
    #error "AUDIO_SAMPLE_INTERVAL_MS too small - will overload system"
#endif

// Memory usage estimation (for monitoring) - GEFIXT
#define ESTIMATED_STATIC_MEMORY (TOTAL_BODY_LEDS * 3 + NUM_EYES * 3 + NUM_MOUTH_LEDS * 3)
#define ESTIMATED_PATTERN_MEMORY (NUM_PATTERNS * 4 + sizeof(IntervalTime) + sizeof(LEDMillis))  // GEFIXT
#define ESTIMATED_PRESET_MEMORY (sizeof(ExtendedPreset) * MAX_PRESETS)

// Total estimated memory usage
#define TOTAL_ESTIMATED_MEMORY (ESTIMATED_STATIC_MEMORY + ESTIMATED_PATTERN_MEMORY + ESTIMATED_PRESET_MEMORY)

#endif // CONFIG_H