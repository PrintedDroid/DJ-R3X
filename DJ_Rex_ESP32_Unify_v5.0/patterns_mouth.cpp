#include "patterns_mouth.h"
#include "audio.h"
#include "helpers.h"

// NEU: Helper function to get the correct color based on split mode
CRGB getMouthColor(int row, int ledInRow) {
    switch (mouthSplitMode) {
        case 1: // Vertical Split
            return (ledInRow < mouthRowLeds[row] / 2) ? getColor(mouthColorIndex) : getColor(mouthColorIndex2);
        case 2: // Horizontal Split
            return (row < MOUTH_ROWS / 2) ? getColor(mouthColorIndex) : getColor(mouthColorIndex2);
        case 3: // Inner/Outer Split
            if (row < 8 && (ledInRow == 0 || ledInRow == 7)) {
                return getColor(mouthColorIndex); // Outer color 1
            } else {
                return getColor(mouthColorIndex2); // Inner color 2
            }
        case 4: // Random Split
            return (random8() < 128) ? getColor(mouthColorIndex) : getColor(mouthColorIndex2);
        case 0: // Off (default to color 1)
        default:
            return getColor(mouthColorIndex);
    }
}


CRGB adjustMouthBrightness(CRGB color, int row, int ledInRow) {
    CRGB adjustedColor = color;
    uint16_t boostFactor = 100; 
    
    if (row < 8 && mouthRowLeds[row] == 8) {
        if (ledInRow == 0 || ledInRow == 7) {
            boostFactor = mouthOuterBoost;
        } 
        else {
            boostFactor = mouthInnerBoost;
        }
    } 
    else if (row >= 8) {
        boostFactor = mouthInnerBoost;
    }
    
    adjustedColor.r = min(255, (uint16_t)(adjustedColor.r * boostFactor) / 100);
    adjustedColor.g = min(255, (uint16_t)(adjustedColor.g * boostFactor) / 100);
    adjustedColor.b = min(255, (uint16_t)(adjustedColor.b * boostFactor) / 100);
    
    return adjustedColor;
}

// NEU: Forward declarations for new patterns
void mouthWave();
void mouthPulse();
void mouthVUMeterHoriz();
void mouthVUMeterVert();
void mouthFrown();
void mouthSparkle();


void updateMouth() {
    // The main switch now includes all 15 patterns
    switch (mouthPattern) {
        case 0: mouthOff(); break;
        case 1: mouthTalk(); break;
        case 2: mouthSmile(); break;
        case 3: mouthAudioReactive(); break;
        case 4: mouthRainbow(); break;
        case 5: mouthDebug(); break;
        case 6: mouthWave(); break;
        case 7: mouthPulse(); break;
        case 8: mouthVUMeterHoriz(); break;
        case 9: mouthVUMeterVert(); break;
        case 10: mouthFrown(); break;
        case 11: mouthSparkle(); break;
        // v5.0: New patterns
        case 12: mouthMatrix(); break;
        case 13: mouthHeartbeat(); break;
        case 14: mouthSpectrum(); break;
    }
}

void mouthOff() {
    fadeToBlackBy(DJLEDs_Mouth, NUM_MOUTH_LEDS, 20);
}

void mouthTalk() {
    static uint8_t talkFrame = 0;
    static unsigned long lastTalkUpdate = 0;
    
    uint16_t talkInterval = map(talkSpeed, 1, 10, 500, 50);
    
    if (millis() - lastTalkUpdate > talkInterval) {
        lastTalkUpdate = millis();
        talkFrame = (talkFrame + 1) % 4;
        
        fill_solid(DJLEDs_Mouth, NUM_MOUTH_LEDS, CRGB::Black);
        
        // Animate mouth opening/closing - using all rows
        switch (talkFrame) {
            case 0: // Closed
                for (int i = 0; i < mouthRowLeds[5]; i++) {
                    CRGB mouthColor = getMouthColor(5, i);
                    mouthColor.fadeToBlackBy(255 - mouthBrightness);
                    DJLEDs_Mouth[mouthRowStart[5] + i] = adjustMouthBrightness(mouthColor, 5, i);
                }
                for (int i = 0; i < mouthRowLeds[6]; i++) {
                    CRGB mouthColor = getMouthColor(6, i);
                    mouthColor.fadeToBlackBy(255 - mouthBrightness);
                    DJLEDs_Mouth[mouthRowStart[6] + i] = adjustMouthBrightness(mouthColor, 6, i);
                }
                break;
            case 1: // Slightly open
            case 2: // Open
            case 3: // Wide open
                {
                    int startRow = (talkFrame == 1) ? 3 : (talkFrame == 2) ? 1 : 0;
                    int endRow = (talkFrame == 1) ? 8 : (talkFrame == 2) ? 10 : 11;
                    for (int row = startRow; row <= endRow; row++) {
                        for (int i = 0; i < mouthRowLeds[row]; i++) {
                            CRGB mouthColor = getMouthColor(row, i);
                            mouthColor.fadeToBlackBy(255 - mouthBrightness);
                            DJLEDs_Mouth[mouthRowStart[row] + i] = adjustMouthBrightness(mouthColor, row, i);
                        }
                    }
                }
                break;
        }
    }
}

void mouthSmile() {
    fill_solid(DJLEDs_Mouth, NUM_MOUTH_LEDS, CRGB::Black);
    
    int startRow = 6 - (smileWidth / 2);
    int endRow = 6 + (smileWidth / 2);
    
    for (int row = startRow; row <= endRow; row++) {
        if (row >= 0 && row < MOUTH_ROWS) {
            int offset = abs(row - 6);
            int startLed = offset;
            int endLed = mouthRowLeds[row] - offset;
            
            for (int i = startLed; i < endLed; i++) {
                if (i >= 0 && i < mouthRowLeds[row]) {
                    CRGB smileColor = getMouthColor(row, i);
                    smileColor.fadeToBlackBy(255 - mouthBrightness);
                    DJLEDs_Mouth[mouthRowStart[row] + i] = adjustMouthBrightness(smileColor, row, i);
                }
            }
        }
    }
}

void mouthAudioReactive() {
    int audio = processAudioLevel();
    
    if (audioMode == AUDIO_OFF || (audioMode != AUDIO_MOUTH_ONLY && audioMode != AUDIO_ALL)) {
        fadeToBlackBy(DJLEDs_Mouth, NUM_MOUTH_LEDS, 20);
        return;
    }
    
    fadeToBlackBy(DJLEDs_Mouth, NUM_MOUTH_LEDS, 20);
    
    int activeRows = map(audio, 0, audioThreshold, 0, MOUTH_ROWS);
    activeRows = constrain(activeRows, 0, MOUTH_ROWS);
    
    CRGB audioColor = CHSV(map(audio, 0, audioThreshold, 0, 255), 255, mouthBrightness);
    
    int centerRow = 5;
    for (int i = 0; i < activeRows; i++) {
        int rowUp = centerRow - (i / 2);
        int rowDown = centerRow + (i / 2) + (i % 2);
        
        if (rowUp >= 0 && rowUp < MOUTH_ROWS) {
            for (int led = 0; led < mouthRowLeds[rowUp]; led++) {
                DJLEDs_Mouth[mouthRowStart[rowUp] + led] = adjustMouthBrightness(audioColor, rowUp, led);
            }
        }
        
        if (rowDown >= 0 && rowDown < MOUTH_ROWS && rowDown != rowUp) {
            for (int led = 0; led < mouthRowLeds[rowDown]; led++) {
                DJLEDs_Mouth[mouthRowStart[rowDown] + led] = adjustMouthBrightness(audioColor, rowDown, led);
            }
        }
    }
}

void mouthRainbow() {
    static uint8_t rainbowOffset = 0;
    
    for (int row = 0; row < MOUTH_ROWS; row++) {
        uint8_t hue = rainbowOffset + (row * 255 / MOUTH_ROWS);
        CRGB rowColor = CHSV(hue, 255, mouthBrightness);
        
        for (int i = 0; i < mouthRowLeds[row]; i++) {
            DJLEDs_Mouth[mouthRowStart[row] + i] = adjustMouthBrightness(rowColor, row, i);
        }
    }
    
    EVERY_N_MILLISECONDS(20) {
        rainbowOffset++;
    }
}

// --- NEUE MUSTER AB HIER ---

void mouthWave() {
    uint8_t speed = map(waveSpeed, 1, 10, 20, 2);
    
    for (int row = 0; row < MOUTH_ROWS; row++) {
        for (int i = 0; i < mouthRowLeds[row]; i++) {
            uint8_t brightness = beatsin8(speed, 0, 255, 0, (row * 16 + i * 16));
            CRGB waveColor = getMouthColor(row, i);
            waveColor.fadeToBlackBy(255 - brightness);
            waveColor.fadeToBlackBy(255 - mouthBrightness);
            DJLEDs_Mouth[mouthRowStart[row] + i] = adjustMouthBrightness(waveColor, row, i);
        }
    }
}

void mouthPulse() {
    uint8_t speed = map(pulseSpeed, 1, 10, 4, 20);
    uint8_t brightness = beatsin8(speed, 64, 255);

    for (int row = 0; row < MOUTH_ROWS; row++) {
        for (int i = 0; i < mouthRowLeds[row]; i++) {
            CRGB pulseColor = getMouthColor(row, i);
            pulseColor.fadeToBlackBy(255 - brightness);
            pulseColor.fadeToBlackBy(255 - mouthBrightness);
            DJLEDs_Mouth[mouthRowStart[row] + i] = adjustMouthBrightness(pulseColor, row, i);
        }
    }
}

void mouthVUMeterHoriz() {
    int audio = processAudioLevel();
    if (audioMode == AUDIO_OFF) audio = 0;
    
    fadeToBlackBy(DJLEDs_Mouth, NUM_MOUTH_LEDS, 40);

    int level = map(audio, 0, audioThreshold, 0, 4); // Map to 4 levels (half of an 8-led row)
    level = constrain(level, 0, 4);

    for (int row = 0; row < MOUTH_ROWS; row++) {
        for (int i = 0; i < level; i++) {
            CRGB vuColor = getMouthColor(row, i);
            vuColor.fadeToBlackBy(255-mouthBrightness);
            // Center outwards
            DJLEDs_Mouth[mouthRowStart[row] + 3 - i] = adjustMouthBrightness(vuColor, row, 3 - i);
            DJLEDs_Mouth[mouthRowStart[row] + 4 + i] = adjustMouthBrightness(vuColor, row, 4 + i);
        }
    }
}

void mouthVUMeterVert() {
    int audio = processAudioLevel();
    if (audioMode == AUDIO_OFF) audio = 0;
    
    fadeToBlackBy(DJLEDs_Mouth, NUM_MOUTH_LEDS, 40);

    int level = map(audio, 0, audioThreshold, 0, MOUTH_ROWS);
    level = constrain(level, 0, MOUTH_ROWS);
    
    // Fill from bottom up
    for (int row = MOUTH_ROWS - 1; row >= MOUTH_ROWS - level; row--) {
        for (int i = 0; i < mouthRowLeds[row]; i++) {
            CRGB vuColor = getMouthColor(row, i);
            vuColor.fadeToBlackBy(255 - mouthBrightness);
            DJLEDs_Mouth[mouthRowStart[row] + i] = adjustMouthBrightness(vuColor, row, i);
        }
    }
}

void mouthFrown() {
    fill_solid(DJLEDs_Mouth, NUM_MOUTH_LEDS, CRGB::Black);

    int startRow = 1;
    int endRow = 5;

    for (int row = startRow; row <= endRow; row++) {
        int offset = 3 - abs(row - 3); // Inverted curve logic
        int startLed = offset;
        int endLed = mouthRowLeds[row] - offset;
        
        for (int i = startLed; i < endLed; i++) {
            if (i >= 0 && i < mouthRowLeds[row]) {
                CRGB frownColor = getMouthColor(row, i);
                frownColor.fadeToBlackBy(255 - mouthBrightness);
                DJLEDs_Mouth[mouthRowStart[row] + i] = adjustMouthBrightness(frownColor, row, i);
            }
        }
    }
}

void mouthSparkle() {
    fadeToBlackBy(DJLEDs_Mouth, NUM_MOUTH_LEDS, 20);
    
    if (random8() < 80) {
        int randLed = random16(NUM_MOUTH_LEDS);
        
        // Find row and ledInRow for the random LED to get the right color
        int row = 0;
        for (int j = 0; j < MOUTH_ROWS; j++) {
            if (randLed < mouthRowStart[j] + mouthRowLeds[j]) {
                row = j;
                break;
            }
        }
        int ledInRow = randLed - mouthRowStart[row];

        CRGB sparkleColor = getMouthColor(row, ledInRow);
        sparkleColor.fadeToBlackBy(255 - mouthBrightness);
        DJLEDs_Mouth[randLed] = adjustMouthBrightness(sparkleColor, row, ledInRow);
    }
}


void mouthDebug() {
    static uint8_t debugMode = 0;
    static unsigned long lastDebugTime = 0;
    
    fill_solid(DJLEDs_Mouth, NUM_MOUTH_LEDS, CRGB::Black);
    
    if (millis() - lastDebugTime > 2000) {
        lastDebugTime = millis();
        debugMode = (debugMode + 1) % 3;
        
        Serial.print(F("Mouth Debug Mode: "));
        switch(debugMode) {
            case 0: Serial.println(F("Outer LEDs (Boosted by mouthOuterBoost)")); break;
            case 1: Serial.println(F("Inner LEDs (Boosted by mouthInnerBoost)")); break;
            case 2: Serial.println(F("All with compensation")); break;
        }
    }
    
    CRGB testColor = CRGB(100, 100, 100);
    
    switch(debugMode) {
        case 0: // Outer
            for (int row = 0; row < 8; row++) {
                DJLEDs_Mouth[mouthRowStart[row] + 0] = adjustMouthBrightness(testColor, row, 0);
                DJLEDs_Mouth[mouthRowStart[row] + 7] = adjustMouthBrightness(testColor, row, 7);
            }
            break;
        case 1: // Inner
            for (int row = 0; row < MOUTH_ROWS; row++) {
                 for (int i = 0; i < mouthRowLeds[row]; i++) {
                    if (row >= 8 || (i > 0 && i < 7)) {
                       DJLEDs_Mouth[mouthRowStart[row] + i] = adjustMouthBrightness(testColor, row, i);
                    }
                }
            }
            break;
        case 2: // All
            for (int row = 0; row < MOUTH_ROWS; row++) {
                for (int i = 0; i < mouthRowLeds[row]; i++) {
                    DJLEDs_Mouth[mouthRowStart[row] + i] = adjustMouthBrightness(testColor, row, i);
                }
            }
            break;
    }
}

// =====================================================
// v5.0 NEW MOUTH PATTERNS
// =====================================================

void mouthMatrix() {
    // Matrix-style falling effect
    static uint8_t matrixDrops[8];  // For each column
    static uint8_t matrixBright[8];
    static unsigned long lastMatrixUpdate = 0;

    if (millis() - lastMatrixUpdate > 80) {
        lastMatrixUpdate = millis();

        // Shift drops down
        for (int col = 0; col < 8; col++) {
            if (matrixBright[col] > 0) {
                matrixDrops[col]++;
                matrixBright[col] = matrixBright[col] > 30 ? matrixBright[col] - 30 : 0;
            }

            // Randomly start new drops
            if (matrixDrops[col] >= MOUTH_ROWS || matrixBright[col] == 0) {
                if (random8() < 40) {
                    matrixDrops[col] = 0;
                    matrixBright[col] = 255;
                }
            }
        }
    }

    fadeToBlackBy(DJLEDs_Mouth, NUM_MOUTH_LEDS, 30);

    CRGB matrixColor = getMouthColor(0, 0);

    for (int col = 0; col < 8; col++) {
        int row = matrixDrops[col];
        if (row < MOUTH_ROWS && matrixBright[col] > 0) {
            int ledIdx = mouthRowStart[row] + min(col, mouthRowLeds[row] - 1);
            CRGB color = matrixColor;
            color.fadeToBlackBy(255 - matrixBright[col]);
            color.fadeToBlackBy(255 - mouthBrightness);
            DJLEDs_Mouth[ledIdx] = adjustMouthBrightness(color, row, col);
        }
    }
}

void mouthHeartbeat() {
    // Heartbeat pulse effect - double pulse
    static uint8_t beatPhase = 0;
    static unsigned long lastBeatUpdate = 0;

    uint16_t beatInterval = 50;

    if (millis() - lastBeatUpdate > beatInterval) {
        lastBeatUpdate = millis();
        beatPhase = (beatPhase + 1) % 40;
    }

    // Create heartbeat pattern (two quick pulses, then pause)
    uint8_t brightness = 0;
    if (beatPhase < 4) {
        brightness = beatPhase * 60;  // First pulse up
    } else if (beatPhase < 8) {
        brightness = 240 - (beatPhase - 4) * 60;  // First pulse down
    } else if (beatPhase < 12) {
        brightness = (beatPhase - 8) * 50;  // Second pulse up
    } else if (beatPhase < 16) {
        brightness = 200 - (beatPhase - 12) * 50;  // Second pulse down
    }
    // Phase 16-40: pause (brightness stays 0)

    for (int row = 0; row < MOUTH_ROWS; row++) {
        for (int i = 0; i < mouthRowLeds[row]; i++) {
            CRGB heartColor = getMouthColor(row, i);
            heartColor.fadeToBlackBy(255 - brightness);
            heartColor.fadeToBlackBy(255 - mouthBrightness);
            DJLEDs_Mouth[mouthRowStart[row] + i] = adjustMouthBrightness(heartColor, row, i);
        }
    }
}

void mouthSpectrum() {
    // Audio spectrum analyzer visualization
    int audio = processAudioLevel();
    if (audioMode == AUDIO_OFF) audio = 0;

    fadeToBlackBy(DJLEDs_Mouth, NUM_MOUTH_LEDS, 40);

    // Create pseudo-spectrum with different frequency bands
    static uint8_t bands[8];
    static uint8_t targets[8];
    static unsigned long lastSpectrumUpdate = 0;

    if (millis() - lastSpectrumUpdate > 30) {
        lastSpectrumUpdate = millis();

        // Update targets based on audio
        for (int i = 0; i < 8; i++) {
            // Simulate different frequency bands with some variation
            int bandLevel = audio + random8(20) - 10;
            bandLevel = constrain(bandLevel, 0, audioThreshold);
            targets[i] = map(bandLevel, 0, audioThreshold, 0, MOUTH_ROWS);

            // Smooth transition
            if (bands[i] < targets[i]) {
                bands[i]++;
            } else if (bands[i] > targets[i]) {
                bands[i]--;
            }
        }
    }

    // Draw spectrum bars
    for (int col = 0; col < 8; col++) {
        int level = bands[col];

        for (int row = MOUTH_ROWS - 1; row >= MOUTH_ROWS - level; row--) {
            if (row >= 0 && row < MOUTH_ROWS && col < mouthRowLeds[row]) {
                // Color based on level (green->yellow->red)
                uint8_t hue = map(MOUTH_ROWS - 1 - row, 0, MOUTH_ROWS, 96, 0);
                CRGB specColor = CHSV(hue, 255, mouthBrightness);
                DJLEDs_Mouth[mouthRowStart[row] + col] = adjustMouthBrightness(specColor, row, col);
            }
        }
    }
}