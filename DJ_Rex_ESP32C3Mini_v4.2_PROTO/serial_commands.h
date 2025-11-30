#ifndef SERIAL_COMMANDS_H
#define SERIAL_COMMANDS_H

#include "config.h"
#include "globals.h"

bool checkSerialCommand();
void processSerialCommand();
void printHelp();
void printCurrentSettings();
void printBlockColors();
void printSystemInfo();

#if ENABLE_MEMORY_MONITORING
void printMemoryStatus();
#endif

#endif