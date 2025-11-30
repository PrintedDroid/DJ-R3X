// event_logger.cpp - v5.0 Event Logging System
#include "event_logger.h"
#include <Arduino.h>

EventLogger eventLogger;

void EventLogger::begin() {
    clear();
    log(EVENT_SYSTEM_START);
    Serial.println(F("Event Logger initialized"));
}

void EventLogger::log(EventType type, uint8_t data) {
    entries[head].timestamp = millis();
    entries[head].type = type;
    entries[head].data = data;

    head = (head + 1) % MAX_LOG_ENTRIES;
    if (count < MAX_LOG_ENTRIES) count++;
}

void EventLogger::printLog() {
    Serial.println(F("=== Event Log ==="));
    Serial.print(F("Entries: "));
    Serial.println(count);

    if (count == 0) {
        Serial.println(F("(empty)"));
        return;
    }

    uint8_t idx = (head - count + MAX_LOG_ENTRIES) % MAX_LOG_ENTRIES;
    for (uint8_t i = 0; i < count; i++) {
        Serial.print(entries[idx].timestamp);
        Serial.print(F("ms: "));

        switch (entries[idx].type) {
            case EVENT_SYSTEM_START:
                Serial.println(F("System Start"));
                break;
            case EVENT_PATTERN_CHANGE:
                Serial.print(F("Pattern -> "));
                Serial.println(entries[idx].data);
                break;
            case EVENT_PRESET_LOAD:
                Serial.print(F("Preset Load #"));
                Serial.println(entries[idx].data);
                break;
            case EVENT_PRESET_SAVE:
                Serial.print(F("Preset Save #"));
                Serial.println(entries[idx].data);
                break;
            case EVENT_ERROR:
                Serial.print(F("Error: "));
                Serial.println(entries[idx].data);
                break;
            case EVENT_MEMORY_WARNING:
                Serial.println(F("Memory Warning"));
                break;
            default:
                Serial.println(F("Unknown"));
        }

        idx = (idx + 1) % MAX_LOG_ENTRIES;
    }
    Serial.println(F("================="));
}

void EventLogger::clear() {
    head = 0;
    count = 0;
}

uint8_t EventLogger::getEntryCount() {
    return count;
}
