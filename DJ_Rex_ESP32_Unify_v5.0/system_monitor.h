// system_monitor.h - v5.0 System Health & Memory Monitoring
#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include "config.h"

class SystemMonitor {
public:
    void begin();
    void update();
    void printStatus();

    uint32_t getFreeHeap();
    uint32_t getMinFreeHeap();
    uint32_t getLoopCounter();
    float getLoopsPerSecond();

    bool isHealthy();
    bool isMemoryLow();
    bool isMemoryCritical();

private:
    unsigned long lastMemoryCheck = 0;
    unsigned long lastHealthCheck = 0;
    unsigned long lastLoopReport = 0;
    uint32_t loopCounter = 0;
    uint32_t lastLoopCount = 0;
    float loopsPerSecond = 0;
    uint8_t consecutiveErrors = 0;
    uint32_t minFreeHeap = 0xFFFFFFFF;

    void checkMemory();
    void checkHealth();
    void updateLoopStats();
};

extern SystemMonitor systemMonitor;

#endif
