#ifndef STARTUP_H
#define STARTUP_H

#include "config.h"
#include "globals.h"

void initializeStartup();
void runStartupSequence();
bool isStartupComplete();

#endif