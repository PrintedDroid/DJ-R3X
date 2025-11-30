#ifndef AUDIO_H
#define AUDIO_H

#include "config.h"
#include "globals.h"

void initializeAudio();
int readAudioLevel();
int processAudioLevel();
void updateAutoGain();

#endif