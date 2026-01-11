#include "audio.h"

// v5.0.1: ADC DC-offset (calibrated at startup)
int adcDCOffset = 2048; // Default, will be calibrated

// v5.0.1: Calibrate ADC DC-offset by averaging mic readings
void calibrateADCOffset() {
    Serial.println(F("Calibrating ADC DC-offset..."));

    long sum = 0;
    const int samples = 200; // 200 samples over ~200ms

    for (int i = 0; i < samples; i++) {
        sum += analogRead(MIC_PIN);
        delay(1); // 1ms between samples
    }

    adcDCOffset = sum / samples;

    Serial.print(F("ADC DC-offset calibrated to: "));
    Serial.println(adcDCOffset);
}

void initializeAudio() {
    // v5.0.1: Calibrate DC offset first
    calibrateADCOffset();

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
        // v5.0.1: Use calibrated DC-offset instead of hardcoded 2048
        audioLevel = abs(reading - adcDCOffset);
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

// v5.0: Main audio update function for FreeRTOS task
void updateAudio() {
    // Process audio level if audio mode is enabled
    if (audioMode != AUDIO_OFF) {
        processAudioLevel();
    }
}