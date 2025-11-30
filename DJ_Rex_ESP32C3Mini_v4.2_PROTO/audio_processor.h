#ifndef AUDIO_PROCESSOR_H
#define AUDIO_PROCESSOR_H

#include "config.h"
#include "globals.h"
#include <freertos/semphr.h>

class AudioProcessor {
private:
    // Raw audio data
    int currentLevel;
    int peakLevel;
    float averageLevel;
    
    // Frequency bands
    float bassLevel;
    float midLevel;
    float trebleLevel;
    
    // Beat detection
    bool beatDetected;
    float beatThreshold;
    unsigned long lastBeatTime;
    uint16_t currentBPM;
    unsigned long beatHistory[10];
    uint8_t beatHistoryIndex;
    
    // Auto gain
    float gainMultiplier;
    int noiseFloor;
    
    // Filters
    float bassFilter;
    float midFilter;
    float trebleFilter;
    
    // Thread safety
    SemaphoreHandle_t dataMutex;

public:
    AudioProcessor();
    ~AudioProcessor();

    void begin();
    void update();
    void resetPeak();
    
    // Getters (Thread-safe)
    int getLevel();
    int getPeakLevel();
    float getAverageLevel();
    float getBassLevel();
    float getMidLevel();
    float getTrebleLevel();
    bool isBeatDetected();
    uint16_t getBPM();
    float getGainMultiplier();
    
    // Audio reactive colors
    CRGB getAudioColor(uint8_t baseColorIndex);
    CRGB getBeatColor();
    uint8_t getAudioBrightness();

private:
    // Internal processing functions
    void processAudio();
    void detectBeat();
    void updateAutoGain();
    void calculateFrequencyBands();
};

extern AudioProcessor audioProcessor;

#endif