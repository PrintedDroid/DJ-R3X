#include "audio.h"

void initializeAudio() {
    // Initialize audio samples
    for (int i = 0; i < 10; i++) {
        audioSamples[i] = 0;
    }
    audioSampleIdx = 0;
    averageAudio = 0;
}

int readAudioLevel() {
    if (millis() - lastAudioRead > 10) {
        int reading = analogRead(MIC_PIN);
        // ESP32 ADC is 12-bit (0-4095), center at ~2048
        audioLevel = abs(reading - 2048);
        lastAudioRead = millis();
        
        // Update auto gain if enabled
        if (audioAutoGain) {
            updateAutoGain();
        }
    }
    return audioLevel;
}

int processAudioLevel() {
    int audio = readAudioLevel();
    
    // Add to samples for averaging
    audioSamples[audioSampleIdx] = audio;
    audioSampleIdx = (audioSampleIdx + 1) % 10;
    
    // Calculate average
    int total = 0;
    for (int i = 0; i < 10; i++) {
        total += audioSamples[i];
    }
    averageAudio = total / 10;
    
    // Apply sensitivity
    audio = map(audio, 0, 2048, 0, audioSensitivity * 100);
    audio = constrain(audio, 0, audioThreshold * 2);
    
    return audio;
}

void updateAutoGain() {
    // Track min/max levels
    if (audioLevel < audioMinLevel) audioMinLevel = audioLevel;
    if (audioLevel > audioMaxLevel) audioMaxLevel = audioLevel;
    
    // Adjust threshold based on dynamic range
    static unsigned long lastGainUpdate = 0;
    if (millis() - lastGainUpdate > 1000) { // Update every second
        lastGainUpdate = millis();
        
        int range = audioMaxLevel - audioMinLevel;
        if (range > 50) { // Minimum range to avoid noise
            audioThreshold = audioMinLevel + (range / 2);
            
            // Slowly decay min/max for adaptation
            audioMinLevel += 10;
            audioMaxLevel -= 10;
            
            // Constrain threshold
            audioThreshold = constrain(audioThreshold, 50, 500);
        }
    }
}