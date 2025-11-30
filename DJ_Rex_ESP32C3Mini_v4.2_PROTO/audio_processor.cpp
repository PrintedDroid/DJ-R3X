#include "audio_processor.h"
#include "helpers.h"

AudioProcessor audioProcessor;

AudioProcessor::AudioProcessor() {
    // Initialize all member variables
    currentLevel = 0;
    peakLevel = 0;
    averageLevel = 0.0f;
    bassLevel = 0.0f;
    midLevel = 0.0f;
    trebleLevel = 0.0f;
    beatDetected = false;
    beatThreshold = BEAT_THRESHOLD;
    lastBeatTime = 0;
    currentBPM = 0;
    beatHistoryIndex = 0;
    gainMultiplier = 1.0f;
    noiseFloor = 50;
    bassFilter = 0.0f;
    midFilter = 0.0f;
    trebleFilter = 0.0f;
    dataMutex = nullptr;

    for (int i = 0; i < 10; i++) {
        beatHistory[i] = 0;
    }
}

AudioProcessor::~AudioProcessor() {
    if (dataMutex != nullptr) {
        vSemaphoreDelete(dataMutex);
    }
}

void AudioProcessor::begin() {
    // Create mutex for thread safety
    dataMutex = xSemaphoreCreateMutex();
    if (dataMutex == nullptr) {
        Serial.println(F("ERROR: Failed to create audio data mutex!"));
    }

    // Establish a baseline noise floor
    long totalNoise = 0;
    for (int i = 0; i < 20; i++) {
        totalNoise += abs(analogRead(MIC_PIN) - 2048);
        delay(5);
    }
    noiseFloor = (totalNoise / 20) + 20; // Average reading + safety margin
    
    Serial.print(F("Audio processor initialized. Noise floor: "));
    Serial.println(noiseFloor);
}

void AudioProcessor::update() {
    // This function is called from a dedicated FreeRTOS task.
    // It acquires a lock, performs all processing, and then releases the lock.
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        processAudio();
        calculateFrequencyBands();
        detectBeat();
        
        if (audioAutoGain) {
            updateAutoGain();
        }
        xSemaphoreGive(dataMutex);
    }
}

void AudioProcessor::processAudio() {
    // This is an internal function, protected by the mutex in update()
    int maxVal = 0;
    int minVal = 4095;
    
    // Fast sample window to find peak-to-peak amplitude
    for (int i = 0; i < 16; i++) {
        int reading = analogRead(MIC_PIN);
        if (reading > maxVal) maxVal = reading;
        if (reading < minVal) minVal = reading;
        delayMicroseconds(50); // Small delay for ADC
    }
    
    // Level is the peak-to-peak amplitude
    int level = maxVal - minVal;

    // Apply noise floor
    if (level < noiseFloor) {
        level = 0;
    } else {
        level -= noiseFloor;
    }
    
    // Apply gain and constrain
    currentLevel = (int)(level * gainMultiplier);
    currentLevel = constrain(currentLevel, 0, 2048);
    
    // Update peak with decay
    if (currentLevel > peakLevel) {
        peakLevel = currentLevel;
    } else {
        peakLevel = peakLevel * 0.98; // Decay peak level
    }
    
    // Update average with exponential moving average
    averageLevel = averageLevel * 0.95f + currentLevel * 0.05f;
}

void AudioProcessor::calculateFrequencyBands() {
    // Simple frequency separation using different time constants (without FFT)
    // Bass - slow response filter
    bassFilter += (currentLevel - bassFilter) * 0.05f;
    
    // Mid - medium response filter
    midFilter += (currentLevel - midFilter) * 0.2f;
    
    // Treble - difference between fast and slow changes
    trebleFilter = abs(currentLevel - averageLevel);
    
    // Convert to 0-100 scale based on audioThreshold
    bassLevel = constrain(map((long)(bassFilter * 1.2), 0, audioThreshold, 0, 100), 0, 100);
    midLevel = constrain(map((long)midFilter, 0, audioThreshold, 0, 100), 0, 100);
    trebleLevel = constrain(map((long)(trebleFilter * 1.5), 0, audioThreshold, 0, 100), 0, 100);
}

void AudioProcessor::detectBeat() {
    beatDetected = false;
    
    // Simple beat detection based on bass energy surpassing a dynamic threshold
    static float avgBassEnergy = 0.0f;
    avgBassEnergy = avgBassEnergy * 0.98f + bassFilter * 0.02f; // Slower average
    
    if (bassFilter > avgBassEnergy * beatThreshold && millis() - lastBeatTime > 150) { // 150ms refractory period (400 BPM max)
        beatDetected = true;
        unsigned long now = millis();
        unsigned long previousBeatTime = beatHistory[(beatHistoryIndex + 9) % 10];
        
        // Calculate BPM
        if (previousBeatTime > 0) {
            float currentInterval = now - previousBeatTime;
            if (currentInterval > 200 && currentInterval < 2000) { // 30-300 BPM
                beatHistory[beatHistoryIndex] = currentInterval;
                beatHistoryIndex = (beatHistoryIndex + 1) % 10;
                
                float totalInterval = 0;
                for(int i=0; i<10; i++) totalInterval += beatHistory[i];
                float avgInterval = totalInterval / 10.0f;
                
                if (avgInterval > 0) {
                    currentBPM = (uint16_t)(60000.0f / avgInterval);
                }
            }
        }
        lastBeatTime = now;
    }
}

void AudioProcessor::updateAutoGain() {
    static float longTermPeak = 0.0f;
    static unsigned long lastGainUpdate = 0;
    
    if (peakLevel > longTermPeak) {
        longTermPeak = peakLevel;
    } else {
        longTermPeak *= 0.999f; // Very slow decay
    }
    
    if (millis() - lastGainUpdate > 500) {
        lastGainUpdate = millis();
        
        if (longTermPeak > 10 && longTermPeak < audioThreshold * 0.4f) {
            gainMultiplier *= 1.05f; // Increase gain
        } else if (longTermPeak > audioThreshold * 0.8f) {
            gainMultiplier *= 0.95f; // Decrease gain
        }
        
        gainMultiplier = constrain(gainMultiplier, 0.5f, 5.0f);
    }
}

// --- Thread-Safe Getters ---

int AudioProcessor::getLevel() {
    int val = 0;
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        val = currentLevel;
        xSemaphoreGive(dataMutex);
    }
    return val;
}

int AudioProcessor::getPeakLevel() {
    int val = 0;
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        val = peakLevel;
        xSemaphoreGive(dataMutex);
    }
    return val;
}

float AudioProcessor::getAverageLevel() {
    float val = 0.0f;
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        val = averageLevel;
        xSemaphoreGive(dataMutex);
    }
    return val;
}

float AudioProcessor::getBassLevel() {
    float val = 0.0f;
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        val = bassLevel;
        xSemaphoreGive(dataMutex);
    }
    return val;
}

float AudioProcessor::getMidLevel() {
    float val = 0.0f;
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        val = midLevel;
        xSemaphoreGive(dataMutex);
    }
    return val;
}

float AudioProcessor::getTrebleLevel() {
    float val = 0.0f;
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        val = trebleLevel;
        xSemaphoreGive(dataMutex);
    }
    return val;
}

bool AudioProcessor::isBeatDetected() {
    bool val = false;
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        val = beatDetected;
        xSemaphoreGive(dataMutex);
    }
    return val;
}

uint16_t AudioProcessor::getBPM() {
    uint16_t val = 0;
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        val = currentBPM;
        xSemaphoreGive(dataMutex);
    }
    return val;
}

float AudioProcessor::getGainMultiplier() {
    float val = 1.0f;
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        val = gainMultiplier;
        xSemaphoreGive(dataMutex);
    }
    return val;
}

void AudioProcessor::resetPeak() {
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        peakLevel = 0;
        xSemaphoreGive(dataMutex);
    }
}

// --- Color Helpers ---

CRGB AudioProcessor::getAudioColor(uint8_t baseColorIndex) {
    CRGB baseColor = getColor(baseColorIndex);
    if (isBeatDetected()) {
        return blend(baseColor, CRGB::White, 200);
    } else {
        uint8_t audioBright = map(getLevel(), 0, audioThreshold, 50, 255);
        baseColor.nscale8(audioBright);
        return baseColor;
    }
}

CRGB AudioProcessor::getBeatColor() {
    static uint8_t beatColorIndex = 0;
    if (isBeatDetected()) {
        beatColorIndex = (beatColorIndex + 1) % NUM_STANDARD_COLORS;
    }
    return getColor(beatColorIndex);
}

uint8_t AudioProcessor::getAudioBrightness() {
    return map(getLevel(), 0, audioThreshold, 0, 255);
}