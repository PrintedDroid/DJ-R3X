#ifndef EVENT_LOGGER_H
#define EVENT_LOGGER_H

#include "config.h"
#include <Arduino.h>

struct LogEntry {
    unsigned long timestamp;
    EventType type;
    String message;
};

class EventLogger {
private:
    LogEntry entries[MAX_LOG_ENTRIES];
    uint8_t writeIndex;
    uint8_t count;
    
public:
    EventLogger();
    void log(EventType type, const String& message);
    void clear();
    LogEntry* getEntry(uint8_t index);
    uint8_t getCount() { return count; }
};

extern EventLogger eventLogger;

#endif