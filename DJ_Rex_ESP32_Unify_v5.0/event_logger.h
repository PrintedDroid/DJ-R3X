// event_logger.h - v5.0 Event Logging System
#ifndef EVENT_LOGGER_H
#define EVENT_LOGGER_H

#include "config.h"

struct LogEntry {
    unsigned long timestamp;
    EventType type;
    uint8_t data;
};

class EventLogger {
public:
    void begin();
    void log(EventType type, uint8_t data = 0);
    void printLog();
    void clear();
    uint8_t getEntryCount();

private:
    LogEntry entries[MAX_LOG_ENTRIES];
    uint8_t head = 0;
    uint8_t count = 0;
};

extern EventLogger eventLogger;

#endif
