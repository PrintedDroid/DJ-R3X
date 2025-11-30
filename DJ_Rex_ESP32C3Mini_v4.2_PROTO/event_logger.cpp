#include "event_logger.h"

EventLogger eventLogger;

EventLogger::EventLogger() {
    writeIndex = 0;
    count = 0;
    clear();
}

void EventLogger::log(EventType type, const String& message) {
    entries[writeIndex].timestamp = millis();
    entries[writeIndex].type = type;
    entries[writeIndex].message = message;
    
    writeIndex = (writeIndex + 1) % MAX_LOG_ENTRIES;
    if (count < MAX_LOG_ENTRIES) {
        count++;
    }
    
    // Also print to serial
    Serial.print(F("["));
    Serial.print(millis() / 1000);
    Serial.print(F("s] "));
    
    switch(type) {
        case EVENT_SYSTEM_START:
            Serial.print(F("SYSTEM: "));
            break;
        case EVENT_PATTERN_CHANGE:
            Serial.print(F("PATTERN: "));
            break;
        case EVENT_PRESET_LOAD:
            Serial.print(F("PRESET: "));
            break;
        case EVENT_ERROR:
            Serial.print(F("ERROR: "));
            break;
        case EVENT_BEAT_DETECTED:
            Serial.print(F("BEAT: "));
            break;
        default:
            Serial.print(F("EVENT: "));
    }
    
    Serial.println(message);
}

void EventLogger::clear() {
    for (int i = 0; i < MAX_LOG_ENTRIES; i++) {
        entries[i].timestamp = 0;
        entries[i].type = EVENT_SYSTEM_START;
        entries[i].message = "";
    }
    writeIndex = 0;
    count = 0;
}

LogEntry* EventLogger::getEntry(uint8_t index) {
    if (index >= count) return nullptr;
    
    int actualIndex = (writeIndex - count + index) % MAX_LOG_ENTRIES;
    if (actualIndex < 0) actualIndex += MAX_LOG_ENTRIES;
    
    return &entries[actualIndex];
}