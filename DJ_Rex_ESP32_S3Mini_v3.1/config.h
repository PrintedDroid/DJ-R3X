#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <FastLED.h>

// Hardware configuration
#define NUM_LEDS_PER_PANEL 20
#define NUM_EYES 2
#define NUM_MOUTH_LEDS 80
#define TOTAL_BODY_LEDS 60

// Pin definitions for ESP32-S3
#define LED_PIN_RIGHT  5
#define LED_PIN_MIDDLE 6
#define LED_PIN_LEFT   7
#define EYES_MOUTH_PIN 8
#define MIC_PIN        1

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
#define NUM_MOUTH_PATTERNS 12

// Color configuration
#define NUM_STANDARD_COLORS 20
#define RANDOM_COLOR_INDEX 19

// Timing
#define FRAMES_PER_SECOND 30
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
#define DECAYTIME 80

// Audio modes
enum AudioMode {
    AUDIO_OFF = 0,
    AUDIO_MOUTH_ONLY = 1,
    AUDIO_BODY_SIDES = 2,
    AUDIO_BODY_ALL = 3,
    AUDIO_ALL = 4
};

// Pattern count
#define NUM_PATTERNS 17

#endif