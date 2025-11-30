#include "status_led.h"
#include <Arduino.h>

#if ENABLE_STATUS_LED

static StatusLEDPattern currentPattern = STATUS_OFF;
static unsigned long lastBlinkTime = 0;
static bool ledState = false;

// Blink patterns (on time, off time in ms)
const uint16_t blinkPatterns[][2] = {
    {0, 0},         // STATUS_OFF - always off
    {100, 100},     // STATUS_STARTUP - fast blink
    {1000, 2000},   // STATUS_NORMAL - slow blink
    {50, 50},       // STATUS_ERROR - very fast blink
    {500, 500},     // STATUS_DEMO - medium blink
    {200, 50},      // STATUS_AUDIO_ACTIVE - quick flash
    {150, 150},     // STATUS_SERIAL_CMD - medium fast blink
    {75, 75},       // STATUS_MEMORY_WARNING - fast blink
    {100, 400},     // STATUS_MUTEX_TIMEOUT - short on, long off
    {200, 200}      // STATUS_PATTERN_ERROR - medium blink
};

void initStatusLED() {
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);
    currentPattern = STATUS_OFF;
    lastBlinkTime = millis();
    ledState = false;
}

void updateStatusLED() {
    if (currentPattern == STATUS_OFF) {
        digitalWrite(STATUS_LED_PIN, LOW);
        return;
    }

    unsigned long currentTime = millis();
    uint16_t onTime = blinkPatterns[currentPattern][0];
    uint16_t offTime = blinkPatterns[currentPattern][1];

    if (ledState) {
        // LED is on, check if it's time to turn off
        if (currentTime - lastBlinkTime >= onTime) {
            ledState = false;
            digitalWrite(STATUS_LED_PIN, LOW);
            lastBlinkTime = currentTime;
        }
    } else {
        // LED is off, check if it's time to turn on
        if (currentTime - lastBlinkTime >= offTime) {
            ledState = true;
            digitalWrite(STATUS_LED_PIN, HIGH);
            lastBlinkTime = currentTime;
        }
    }
}

void setStatusLED(StatusLEDPattern pattern) {
    if (pattern != currentPattern) {
        currentPattern = pattern;
        lastBlinkTime = millis();
        ledState = false;

        // Immediately update LED state
        if (pattern == STATUS_OFF) {
            digitalWrite(STATUS_LED_PIN, LOW);
        }
    }
}

StatusLEDPattern getStatusLED() {
    return currentPattern;
}

#else

// Status LED disabled - empty stub functions
static StatusLEDPattern currentPattern = STATUS_OFF;

void initStatusLED() {
    // Status LED not present in hardware
}

void updateStatusLED() {
    // Status LED not present in hardware
}

void setStatusLED(StatusLEDPattern pattern) {
    currentPattern = pattern;  // Track state even if LED not present
}

StatusLEDPattern getStatusLED() {
    return currentPattern;
}

#endif
