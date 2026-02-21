// system_monitor.cpp - v5.0 System Health & Memory Monitoring
#include "system_monitor.h"
#include "event_logger.h"
#include "globals.h"
#include <Arduino.h>
#include <esp_system.h>

SystemMonitor systemMonitor;

// Safe pattern to switch to on critical errors (0 = LEDs Off)
#define SAFE_PATTERN 0
#define AUTO_RESTART_THRESHOLD 10

void SystemMonitor::begin() {
    minFreeHeap = ESP.getFreeHeap();
    lastMemoryCheck = millis();
    lastHealthCheck = millis();
    lastLoopReport = millis();
    loopCounter = 0;
    consecutiveErrors = 0;

    Serial.println(F("System Monitor initialized"));
    printStatus();
}

void SystemMonitor::update() {
    loopCounter++;

    unsigned long now = millis();

    // Update loop statistics every second
    if (now - lastLoopReport >= 1000) {
        updateLoopStats();
        lastLoopReport = now;
    }

    // Memory check
    #if ENABLE_MEMORY_MONITORING
    if (now - lastMemoryCheck >= MEMORY_CHECK_INTERVAL_MS) {
        checkMemory();
        lastMemoryCheck = now;
    }
    #endif

    // Health check
    #if ENABLE_HEALTH_CHECK
    if (now - lastHealthCheck >= HEALTH_CHECK_INTERVAL_MS) {
        checkHealth();
        lastHealthCheck = now;
    }
    #endif
}

void SystemMonitor::updateLoopStats() {
    loopsPerSecond = loopCounter - lastLoopCount;
    lastLoopCount = loopCounter;
}

void SystemMonitor::checkMemory() {
    uint32_t freeHeap = ESP.getFreeHeap();

    if (freeHeap < minFreeHeap) {
        minFreeHeap = freeHeap;
    }

    if (freeHeap < MEMORY_CRITICAL_THRESHOLD) {
        Serial.print(F("CRITICAL: Memory very low: "));
        Serial.print(freeHeap);
        Serial.println(F(" bytes"));
        eventLogger.log(EVENT_MEMORY_WARNING, 2);
        consecutiveErrors++;
    } else if (freeHeap < MEMORY_WARNING_THRESHOLD) {
        Serial.print(F("WARNING: Memory low: "));
        Serial.print(freeHeap);
        Serial.println(F(" bytes"));
        eventLogger.log(EVENT_MEMORY_WARNING, 1);
    }
}

void SystemMonitor::checkHealth() {
    bool healthy = true;

    // Check loop performance
    if (loopsPerSecond < 10) {
        Serial.println(F("WARNING: Low loop performance"));
        healthy = false;
    }

    // Check memory
    if (isMemoryCritical()) {
        healthy = false;
    }

    if (healthy) {
        consecutiveErrors = 0;
    } else {
        consecutiveErrors++;

        // Auto-recovery: switch to safe pattern on critical errors
        if (consecutiveErrors >= MAX_CONSECUTIVE_ERRORS) {
            Serial.println(F("CRITICAL: Switching to safe mode!"));
            eventLogger.log(EVENT_ERROR, consecutiveErrors);

            // Switch to safe pattern (LEDs Off)
            currentPattern = SAFE_PATTERN;
            demoMode = false;
            playlistActive = false;

            Serial.println(F("Safe mode activated - LEDs Off"));
        }

        // Auto-restart after too many errors
        if (consecutiveErrors >= AUTO_RESTART_THRESHOLD) {
            Serial.println(F("CRITICAL: System restart in 3 seconds..."));
            delay(3000);
            ESP.restart();
        }
    }
}

void SystemMonitor::printStatus() {
    Serial.println(F("=== System Status ==="));
    Serial.print(F("Free Heap: "));
    Serial.print(getFreeHeap());
    Serial.println(F(" bytes"));
    Serial.print(F("Min Free Heap: "));
    Serial.print(getMinFreeHeap());
    Serial.println(F(" bytes"));
    Serial.print(F("Loops/sec: "));
    Serial.println(getLoopsPerSecond(), 1);
    Serial.print(F("Health: "));
    Serial.println(isHealthy() ? "OK" : "WARNING");
    Serial.println(F("===================="));
}

uint32_t SystemMonitor::getFreeHeap() {
    return ESP.getFreeHeap();
}

uint32_t SystemMonitor::getMinFreeHeap() {
    return minFreeHeap;
}

uint32_t SystemMonitor::getLoopCounter() {
    return loopCounter;
}

float SystemMonitor::getLoopsPerSecond() {
    return loopsPerSecond;
}

bool SystemMonitor::isHealthy() {
    return consecutiveErrors < MAX_CONSECUTIVE_ERRORS && !isMemoryCritical();
}

bool SystemMonitor::isMemoryLow() {
    return ESP.getFreeHeap() < MEMORY_WARNING_THRESHOLD;
}

bool SystemMonitor::isMemoryCritical() {
    return ESP.getFreeHeap() < MEMORY_CRITICAL_THRESHOLD;
}
