#ifndef AUDIO_H
#define AUDIO_H

#include "config.h"
#include "globals.h"

void initializeAudio();
int readAudioLevel();
int processAudioLevel();
void updateAutoGain();

// v5.0: FreeRTOS audio task function
void updateAudio();

#endif