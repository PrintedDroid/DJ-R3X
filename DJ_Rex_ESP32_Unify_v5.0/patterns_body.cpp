#include "patterns_body.h"
#include "helpers.h"
#include "audio.h"

// Pattern list definition
SimplePatternList gPatterns = {
    LEDsOff,           // 0
    RandomBlocks,      // 1
    SolidColor,        // 2
    ShortCircuit,      // 3
    ConfettiRedWhite,  // 4
    rainbow,           // 5
    rainbowWithGlitter,// 6
    confetti,          // 7
    juggle,            // 8
    audioSync,         // 9
    SolidFlash,        // 10
    knightRider,       // 11
    breathing,         // 12
    matrixRain,        // 13
    strobePattern,     // 14
    audioVUMeter,      // 15
    CustomBlockSequence, // 16
    // v5.0: New patterns
    plasmaPattern,     // 17
    firePattern,       // 18
    twinklePattern     // 19
};

void LEDsOff() {
    fadeToBlackBy(DJLEDs_Right, NUM_LEDS_PER_PANEL, 5);
    fadeToBlackBy(DJLEDs_Middle, NUM_LEDS_PER_PANEL, 5);
    fadeToBlackBy(DJLEDs_Left, NUM_LEDS_PER_PANEL, 5);
    fadeToBlackBy(DJLEDs_Eyes, NUM_EYES, 5);
    fadeToBlackBy(DJLEDs_Mouth, NUM_MOUTH_LEDS, 5);
}

void RandomBlocks() {
    for (uint8_t panel = 0; panel < 3; panel++) {
        CRGB* leds = getLEDArray(panel);
        
        // Side LEDs
        for (uint8_t i = 0; i < SIDE_LEDS_COUNT; i++) {
            uint8_t timingIdx = getTimingIndex(panel, i);
            
            if (!LEDOn[timingIdx]) {
                leds[i].fadeToBlackBy(fadeSpeed);
            }
            
            if (millis() - LEDMillis[timingIdx] > IntervalTime[timingIdx]) {
                if (!LEDOn[timingIdx]) {
                    leds[i] = getSideLEDColor();
                    IntervalTime[timingIdx] = getRandomTimingWithRate(sideMinTime, sideMaxTime, sideBlinkRate);
                    LEDMillis[timingIdx] = millis();
                    LEDOn[timingIdx] = 1;
                } else {
                    IntervalTime[timingIdx] = getRandomTimingWithRate(sideMinTime, sideMaxTime + 500, sideBlinkRate);
                    LEDMillis[timingIdx] = millis();
                    LEDOn[timingIdx] = 0;
                }
            }
        }
        
        // Blocks
        uint8_t blockStarts[3] = {BLOCK1_START, BLOCK2_START, BLOCK3_START};
        
        for (uint8_t blockNum = 0; blockNum < 3; blockNum++) {
            uint8_t blockStart = blockStarts[blockNum];
            uint8_t timingIdx = getTimingIndex(panel, blockStart);
            uint8_t globalBlockIndex = getGlobalBlockIndex(panel, blockNum);
            
            if (!LEDOn[timingIdx]) {
                fadeBlock(panel, blockStart, fadeSpeed);
            }
            
            if (millis() - LEDMillis[timingIdx] > IntervalTime[timingIdx]) {
                if (!LEDOn[timingIdx]) {
                    CRGB blockColor = getBlockColor(globalBlockIndex);
                    setBlock(panel, blockStart, blockColor);
                    IntervalTime[timingIdx] = getRandomTimingWithRate(blockMinTime, blockMaxTime, blockBlinkRate);
                    LEDMillis[timingIdx] = millis();
                    LEDOn[timingIdx] = 1;
                } else {
                    IntervalTime[timingIdx] = getRandomTimingWithRate(blockMinTime, blockMaxTime + 500, blockBlinkRate);
                    LEDMillis[timingIdx] = millis();
                    LEDOn[timingIdx] = 0;
                }
            }
        }
    }
}

void SolidColor() {
    if (solidMode == 0) {
        CRGB color = getColor(solidColorIndex);
        fill_solid(DJLEDs_Right, NUM_LEDS_PER_PANEL, color);
        fill_solid(DJLEDs_Middle, NUM_LEDS_PER_PANEL, color);
        fill_solid(DJLEDs_Left, NUM_LEDS_PER_PANEL, color);
    } else {
        CRGB selectedColor = getColor(solidColorIndex);
        
        for (uint8_t panel = 0; panel < 3; panel++) {
            CRGB* leds = getLEDArray(panel);
            
            for (uint8_t i = 0; i < SIDE_LEDS_COUNT; i++) {
                uint8_t timingIdx = getTimingIndex(panel, i);
                
                if (!LEDOn[timingIdx]) {
                    leds[i].fadeToBlackBy(fadeSpeed);
                }
                
                if (millis() - LEDMillis[timingIdx] > IntervalTime[timingIdx]) {
                    if (!LEDOn[timingIdx]) {
                        leds[i] = selectedColor;
                        IntervalTime[timingIdx] = getRandomTiming(sideMinTime, sideMaxTime);
                        LEDMillis[timingIdx] = millis();
                        LEDOn[timingIdx] = 1;
                    } else {
                        IntervalTime[timingIdx] = getRandomTiming(sideMinTime, sideMaxTime + 500);
                        LEDMillis[timingIdx] = millis();
                        LEDOn[timingIdx] = 0;
                    }
                }
            }
            
            uint8_t blockStarts[3] = {BLOCK1_START, BLOCK2_START, BLOCK3_START};
            
            for (uint8_t blockNum = 0; blockNum < 3; blockNum++) {
                uint8_t blockStart = blockStarts[blockNum];
                uint8_t timingIdx = getTimingIndex(panel, blockStart);
                
                if (!LEDOn[timingIdx]) {
                    fadeBlock(panel, blockStart, fadeSpeed);
                }
                
                if (millis() - LEDMillis[timingIdx] > IntervalTime[timingIdx]) {
                    if (!LEDOn[timingIdx]) {
                        setBlock(panel, blockStart, selectedColor);
                        IntervalTime[timingIdx] = getRandomTiming(blockMinTime, blockMaxTime);
                        LEDMillis[timingIdx] = millis();
                        LEDOn[timingIdx] = 1;
                    } else {
                        IntervalTime[timingIdx] = getRandomTiming(blockMinTime, blockMaxTime + 500);
                        LEDMillis[timingIdx] = millis();
                        LEDOn[timingIdx] = 0;
                    }
                }
            }
        }
    }
}

void ShortCircuit() {
    if (millis() - FadeMillis > FadeInterval) {
        CRGB sparkColor = getColor(shortColorIndex);
        
        if (random8() < 150) {
            DJLEDs_Right[random16(NUM_LEDS_PER_PANEL)] += sparkColor;
        }
        if (random8() < 150) {
            DJLEDs_Middle[random16(NUM_LEDS_PER_PANEL)] += sparkColor;
        }
        if (random8() < 150) {
            DJLEDs_Left[random16(NUM_LEDS_PER_PANEL)] += sparkColor;
        }
        
        DecayTime--;
        FadeInterval += 4;
        FadeMillis = millis();
    }

    if (DecayTime == 0) {
        DecayTime = DECAYTIME;
        if (!demoMode) {       
            currentPattern = 0;
        }
        FadeInterval = 0;
    }
    
    fadeToBlackBy(DJLEDs_Right, NUM_LEDS_PER_PANEL, fadeSpeed);
    fadeToBlackBy(DJLEDs_Middle, NUM_LEDS_PER_PANEL, fadeSpeed);
    fadeToBlackBy(DJLEDs_Left, NUM_LEDS_PER_PANEL, fadeSpeed);
}

void ConfettiRedWhite() {
    fadeToBlackBy(DJLEDs_Right, NUM_LEDS_PER_PANEL, fadeSpeed);
    fadeToBlackBy(DJLEDs_Middle, NUM_LEDS_PER_PANEL, fadeSpeed);
    fadeToBlackBy(DJLEDs_Left, NUM_LEDS_PER_PANEL, fadeSpeed);

    CRGB color1 = getColor(confettiColor1);
    CRGB color2 = getColor(confettiColor2);
    
    if (random8() < 128) {
        DJLEDs_Right[random16(NUM_LEDS_PER_PANEL)] += color1;
        DJLEDs_Middle[random16(NUM_LEDS_PER_PANEL)] += color1;
        DJLEDs_Left[random16(NUM_LEDS_PER_PANEL)] += color1;
    } else {
        DJLEDs_Right[random16(NUM_LEDS_PER_PANEL)] += color2;
        DJLEDs_Middle[random16(NUM_LEDS_PER_PANEL)] += color2;
        DJLEDs_Left[random16(NUM_LEDS_PER_PANEL)] += color2;
    }
}

void rainbow() {
    fill_rainbow(DJLEDs_Right, NUM_LEDS_PER_PANEL, gHue, 7);
    fill_rainbow(DJLEDs_Middle, NUM_LEDS_PER_PANEL, gHue, 7);
    fill_rainbow(DJLEDs_Left, NUM_LEDS_PER_PANEL, gHue, 7);
}

void rainbowWithGlitter() {
    rainbow();
    addGlitter(80);
}

void addGlitter(fract8 chanceOfGlitter) {
    if (random8() < chanceOfGlitter) {
        DJLEDs_Right[random16(NUM_LEDS_PER_PANEL)] += CRGB::White;
    }
    if (random8() < chanceOfGlitter) {
        DJLEDs_Middle[random16(NUM_LEDS_PER_PANEL)] += CRGB::White;
    }
    if (random8() < chanceOfGlitter) {
        DJLEDs_Left[random16(NUM_LEDS_PER_PANEL)] += CRGB::White;
    }
}

void confetti() {
    fadeToBlackBy(DJLEDs_Right, NUM_LEDS_PER_PANEL, fadeSpeed);
    fadeToBlackBy(DJLEDs_Middle, NUM_LEDS_PER_PANEL, fadeSpeed);
    fadeToBlackBy(DJLEDs_Left, NUM_LEDS_PER_PANEL, fadeSpeed);

    DJLEDs_Right[random16(NUM_LEDS_PER_PANEL)] += CHSV(gHue + random8(64), 200, 255);
    DJLEDs_Middle[random16(NUM_LEDS_PER_PANEL)] += CHSV(gHue + random8(64), 200, 255);
    DJLEDs_Left[random16(NUM_LEDS_PER_PANEL)] += CHSV(gHue + random8(64), 200, 255);
}

void juggle() {
    fadeToBlackBy(DJLEDs_Right, NUM_LEDS_PER_PANEL, 20);
    fadeToBlackBy(DJLEDs_Middle, NUM_LEDS_PER_PANEL, 20);
    fadeToBlackBy(DJLEDs_Left, NUM_LEDS_PER_PANEL, 20);

    byte dothue = 0;
    for (int i = 0; i < 8; i++) {
        DJLEDs_Right[beatsin16(i + 7, 0, NUM_LEDS_PER_PANEL - 1)] |= CHSV(dothue, 200, 255);
        DJLEDs_Middle[beatsin16(i + 7, 0, NUM_LEDS_PER_PANEL - 1)] |= CHSV(dothue, 200, 255);
        DJLEDs_Left[beatsin16(i + 7, 0, NUM_LEDS_PER_PANEL - 1)] |= CHSV(dothue, 200, 255);
        dothue += 32;
    }
}

void audioSync() {
    int audio = processAudioLevel();
    
    // Check audio mode for body panels
    if (audioMode == AUDIO_OFF || audioMode == AUDIO_MOUTH_ONLY) {
        // Clear panels and return
        fadeToBlackBy(DJLEDs_Right, NUM_LEDS_PER_PANEL, 20);
        fadeToBlackBy(DJLEDs_Middle, NUM_LEDS_PER_PANEL, 20);
        fadeToBlackBy(DJLEDs_Left, NUM_LEDS_PER_PANEL, 20);
        return;
    }
    
    int numLEDs = map(audio, 0, audioThreshold, 0, SIDE_LEDS_COUNT);
    numLEDs = constrain(numLEDs, 0, SIDE_LEDS_COUNT);
    
    fadeToBlackBy(DJLEDs_Right, NUM_LEDS_PER_PANEL, 20);
    fadeToBlackBy(DJLEDs_Middle, NUM_LEDS_PER_PANEL, 20);
    fadeToBlackBy(DJLEDs_Left, NUM_LEDS_PER_PANEL, 20);
    
    CRGB audioColor = CHSV(gHue, 255, 255);
    
    // Light up side LEDs
    for (int i = 0; i < numLEDs; i++) {
        DJLEDs_Right[i] = audioColor;
        DJLEDs_Middle[i] = audioColor;
        DJLEDs_Left[i] = audioColor;
    }
    
    // Add blocks if audio mode includes all body
    if (audioMode == AUDIO_BODY_ALL || audioMode == AUDIO_ALL) {
        if (audio > audioThreshold * 0.7) {
            for (int panel = 0; panel < 3; panel++) {
                if (audio > audioThreshold * 0.9) {
                    setBlock(panel, BLOCK3_START, audioColor);
                }
                if (audio > audioThreshold * 0.8) {
                    setBlock(panel, BLOCK2_START, audioColor);
                }
                setBlock(panel, BLOCK1_START, audioColor);
            }
        }
    }
    
    // Add sparkle on high levels
    if (audio > audioThreshold * 0.8) {
        for (int panel = 0; panel < 3; panel++) {
            CRGB* leds = getLEDArray(panel);
            if (random8() < 50) {
                leds[random8(SIDE_LEDS_COUNT)] += CRGB::White;
            }
        }
    }
}

void SolidFlash() {
    uint16_t flashInterval = map(flashSpeed, 1, 10, 1000, 100);
    
    if (millis() - lastFlashTime >= flashInterval) {
        lastFlashTime = millis();
        flashState = !flashState;
    }
    
    CRGB displayColor = flashState ? getColor(flashColorIndex) : CRGB::Black;
    
    fill_solid(DJLEDs_Right, NUM_LEDS_PER_PANEL, displayColor);
    fill_solid(DJLEDs_Middle, NUM_LEDS_PER_PANEL, displayColor);
    fill_solid(DJLEDs_Left, NUM_LEDS_PER_PANEL, displayColor);
}

void knightRider() {
    fadeToBlackBy(DJLEDs_Right, NUM_LEDS_PER_PANEL, 20);
    fadeToBlackBy(DJLEDs_Middle, NUM_LEDS_PER_PANEL, 20);
    fadeToBlackBy(DJLEDs_Left, NUM_LEDS_PER_PANEL, 20);
    
    knightInterval = map(effectSpeed, 1, 255, 200, 30);
    
    if (millis() - knightMillis > knightInterval) {
        knightMillis = millis();
        if (knightDir) {
            knightPos++;
            if (knightPos >= SIDE_LEDS_COUNT - 1) knightDir = false;
        } else {
            knightPos--;
            if (knightPos == 0) knightDir = true;
        }
    }
    
    CRGB color = getColor(knightColorIndex);
    DJLEDs_Right[knightPos] = color;
    DJLEDs_Middle[knightPos] = color;
    DJLEDs_Left[knightPos] = color;
    
    if (knightPos > 0) {
        DJLEDs_Right[knightPos - 1] = color; DJLEDs_Right[knightPos - 1].fadeToBlackBy(128);
        DJLEDs_Middle[knightPos - 1] = color; DJLEDs_Middle[knightPos - 1].fadeToBlackBy(128);
        DJLEDs_Left[knightPos - 1] = color; DJLEDs_Left[knightPos - 1].fadeToBlackBy(128);
    }
    if (knightPos < SIDE_LEDS_COUNT - 1) {
        DJLEDs_Right[knightPos + 1] = color; DJLEDs_Right[knightPos + 1].fadeToBlackBy(128);
        DJLEDs_Middle[knightPos + 1] = color; DJLEDs_Middle[knightPos + 1].fadeToBlackBy(128);
        DJLEDs_Left[knightPos + 1] = color; DJLEDs_Left[knightPos + 1].fadeToBlackBy(128);
    }
}

void breathing() {
    uint16_t speed = map(effectSpeed, 1, 255, 10, 2);
    
    if (millis() - breathingMillis > speed) {
        breathingMillis = millis();
        if (breathingUp) {
            breathingBright += 2;
            if (breathingBright >= 255) { breathingBright = 255; breathingUp = false; }
        } else {
            breathingBright -= 2;
            if (breathingBright <= 10) { breathingBright = 10; breathingUp = true; }
        }
    }
    
    CRGB color = getColor(breathingColorIndex);
    color.fadeToBlackBy(255 - breathingBright);
    
    fill_solid(DJLEDs_Right, NUM_LEDS_PER_PANEL, color);
    fill_solid(DJLEDs_Middle, NUM_LEDS_PER_PANEL, color);
    fill_solid(DJLEDs_Left, NUM_LEDS_PER_PANEL, color);
}

void matrixRain() {
    uint16_t speed = map(effectSpeed, 1, 255, 150, 20);
    
    if (millis() - matrixMillis > speed) {
        matrixMillis = millis();
        
        for (int panel = 0; panel < 3; panel++) {
            CRGB* leds = getLEDArray(panel);
            
            for (int led = SIDE_LEDS_COUNT - 1; led > 0; led--) {
                matrixDrops[panel][led] = matrixDrops[panel][led - 1];
                matrixBright[panel][led] = matrixBright[panel][led - 1];
            }
            
            if (random8() < 50) {
                matrixDrops[panel][0] = 0;
                matrixBright[panel][0] = 255;
            } else {
                matrixDrops[panel][0] = 255;
                matrixBright[panel][0] = 0;
            }
            
            for (int i = 0; i < SIDE_LEDS_COUNT; i++) leds[i] = CRGB::Black;
            
            CRGB color = getColor(matrixColorIndex);
            for (int led = 0; led < SIDE_LEDS_COUNT; led++) {
                if (matrixDrops[panel][led] == 0) {
                    leds[led] = color;
                    leds[led].fadeToBlackBy(255 - matrixBright[panel][led]);
                    if (matrixBright[panel][led] > 30) {
                        matrixBright[panel][led] -= 30;
                    } else {
                        matrixDrops[panel][led] = 255;
                        matrixBright[panel][led] = 0;
                    }
                }
            }
            
            for (int i = SIDE_LEDS_COUNT; i < NUM_LEDS_PER_PANEL; i++) leds[i] = CRGB::Black;
        }
    }
}

void strobePattern() {
    strobeInterval = map(effectSpeed, 1, 255, 300, 50);
    
    if (millis() - strobeMillis > strobeInterval) {
        strobeMillis = millis();
        strobeState = !strobeState;
    }
    
    CRGB color = strobeState ? getColor(strobeColorIndex) : CRGB::Black;
    
    fill_solid(DJLEDs_Right, NUM_LEDS_PER_PANEL, color);
    fill_solid(DJLEDs_Middle, NUM_LEDS_PER_PANEL, color);
    fill_solid(DJLEDs_Left, NUM_LEDS_PER_PANEL, color);
}

void audioVUMeter() {
    int audio = processAudioLevel();
    
    // Check audio mode
    if (audioMode == AUDIO_OFF || audioMode == AUDIO_MOUTH_ONLY) {
        // Clear panels and return
        fill_solid(DJLEDs_Right, NUM_LEDS_PER_PANEL, CRGB::Black);
        fill_solid(DJLEDs_Middle, NUM_LEDS_PER_PANEL, CRGB::Black);
        fill_solid(DJLEDs_Left, NUM_LEDS_PER_PANEL, CRGB::Black);
        return;
    }
    
    fill_solid(DJLEDs_Right, NUM_LEDS_PER_PANEL, CRGB::Black);
    fill_solid(DJLEDs_Middle, NUM_LEDS_PER_PANEL, CRGB::Black);
    fill_solid(DJLEDs_Left, NUM_LEDS_PER_PANEL, CRGB::Black);
    
    int vuLevel = constrain(map(averageAudio, 0, audioThreshold, 0, SIDE_LEDS_COUNT), 0, SIDE_LEDS_COUNT);
    
    for (int panel = 0; panel < 3; panel++) {
        CRGB* leds = getLEDArray(panel);
        for (int i = 0; i < vuLevel; i++) {
            int ledIdx = SIDE_LEDS_COUNT - 1 - i;
            CRGB color = (i < SIDE_LEDS_COUNT/3) ? CRGB::Green : 
                         (i < (SIDE_LEDS_COUNT*2)/3) ? CRGB::Yellow : CRGB::Red;
            leds[ledIdx] = color;
        }
        
        // Peak indicators on blocks if mode allows
        if ((audioMode == AUDIO_BODY_ALL || audioMode == AUDIO_ALL) && averageAudio > audioThreshold * 0.8) {
            setBlock(panel, BLOCK1_START, CRGB::White);
            setBlock(panel, BLOCK2_START, CRGB::White);
            setBlock(panel, BLOCK3_START, CRGB::White);
        }
    }
    
    if (audio > audioThreshold * 0.9 && random8() < 100) {
        getLEDArray(random8(3))[random8(SIDE_LEDS_COUNT)] += CRGB::White;
    }
}

void CustomBlockSequence() {
    // Define the block colors for the sequence
    // 0-8 mapping to requested pattern (1-9)
    static const uint8_t sequenceColors[9] = {
        3,  // Block 0 (1) = White
        2,  // Block 1 (2) = Blue
        2,  // Block 2 (3) = Blue
        3,  // Block 3 (4) = White
        2,  // Block 4 (5) = Blue
        2,  // Block 5 (6) = Blue
        2,  // Block 6 (7) = Blue
        2,  // Block 7 (8) = Blue
        3   // Block 8 (9) = White
    };
    
    // Handle side LEDs with Red, White, Blue random
    for (uint8_t panel = 0; panel < 3; panel++) {
        CRGB* leds = getLEDArray(panel);
        
        // Side LEDs - Random Red/White/Blue
        for (uint8_t i = 0; i < SIDE_LEDS_COUNT; i++) {
            uint8_t timingIdx = getTimingIndex(panel, i);
            
            if (!LEDOn[timingIdx]) {
                leds[i].fadeToBlackBy(fadeSpeed);
            }
            
            if (millis() - LEDMillis[timingIdx] > IntervalTime[timingIdx]) {
                if (!LEDOn[timingIdx]) {
                    // Random between Red(0), Blue(2), White(3)
                    uint8_t colorChoice = random8(3);
                    uint8_t colorIndex = (colorChoice == 0) ? 0 : (colorChoice == 1) ? 2 : 3;
                    leds[i] = getColor(colorIndex);
                    IntervalTime[timingIdx] = getRandomTimingWithRate(sideMinTime, sideMaxTime, sideBlinkRate);
                    LEDMillis[timingIdx] = millis();
                    LEDOn[timingIdx] = 1;
                } else {
                    IntervalTime[timingIdx] = getRandomTimingWithRate(sideMinTime, sideMaxTime + 500, sideBlinkRate);
                    LEDMillis[timingIdx] = millis();
                    LEDOn[timingIdx] = 0;
                }
            }
        }
        
        // Handle blocks with the custom sequence
        uint8_t blockStarts[3] = {BLOCK1_START, BLOCK2_START, BLOCK3_START};
        
        for (uint8_t blockNum = 0; blockNum < 3; blockNum++) {
            uint8_t blockStart = blockStarts[blockNum];
            uint8_t timingIdx = getTimingIndex(panel, blockStart);
            uint8_t globalBlockIndex = getGlobalBlockIndex(panel, blockNum);
            
            if (!LEDOn[timingIdx]) {
                fadeBlock(panel, blockStart, fadeSpeed);
            }
            
            if (millis() - LEDMillis[timingIdx] > IntervalTime[timingIdx]) {
                if (!LEDOn[timingIdx]) {
                    // Use the sequence color for this block
                    CRGB blockColor = getColor(sequenceColors[globalBlockIndex]);
                    setBlock(panel, blockStart, blockColor);
                    IntervalTime[timingIdx] = getRandomTimingWithRate(blockMinTime, blockMaxTime, blockBlinkRate);
                    LEDMillis[timingIdx] = millis();
                    LEDOn[timingIdx] = 1;
                } else {
                    IntervalTime[timingIdx] = getRandomTimingWithRate(blockMinTime, blockMaxTime + 500, blockBlinkRate);
                    LEDMillis[timingIdx] = millis();
                    LEDOn[timingIdx] = 0;
                }
            }
        }
    }
}

// =====================================================
// v5.0 NEW PATTERNS
// =====================================================

void plasmaPattern() {
    // Flowing plasma effect using sin waves
    static uint16_t plasmaTime = 0;
    plasmaTime += effectSpeed / 4;

    for (int panel = 0; panel < 3; panel++) {
        CRGB* leds = getLEDArray(panel);

        for (int i = 0; i < NUM_LEDS_PER_PANEL; i++) {
            // Multiple overlapping sin waves for plasma effect
            uint8_t hue = sin8(i * 15 + plasmaTime / 2) +
                          sin8(i * 7 - plasmaTime / 3) +
                          sin8(panel * 50 + plasmaTime / 4);

            uint8_t brightness = sin8(i * 10 + plasmaTime / 5) / 2 + 127;

            leds[i] = CHSV(hue + gHue, 255, brightness);
        }
    }
}

void firePattern() {
    // Fire simulation effect
    static uint8_t heat[3][NUM_LEDS_PER_PANEL];

    for (int panel = 0; panel < 3; panel++) {
        CRGB* leds = getLEDArray(panel);

        // Cool down every cell a little
        for (int i = 0; i < NUM_LEDS_PER_PANEL; i++) {
            heat[panel][i] = qsub8(heat[panel][i], random8(0, ((55 * 10) / NUM_LEDS_PER_PANEL) + 2));
        }

        // Heat from each cell drifts 'up' and diffuses a little
        for (int k = NUM_LEDS_PER_PANEL - 1; k >= 2; k--) {
            heat[panel][k] = (heat[panel][k - 1] + heat[panel][k - 2] + heat[panel][k - 2]) / 3;
        }

        // Randomly ignite new 'sparks' of heat near the bottom
        if (random8() < 120) {
            int y = random8(3);
            heat[panel][y] = qadd8(heat[panel][y], random8(160, 255));
        }

        // Map from heat cells to LED colors
        for (int j = 0; j < NUM_LEDS_PER_PANEL; j++) {
            // Scale the heat value from 0-255 down to 0-240 for best color values
            uint8_t colorindex = scale8(heat[panel][j], 240);
            leds[j] = HeatColor(colorindex);
        }
    }
}

void twinklePattern() {
    // Random twinkling stars effect
    fadeToBlackBy(DJLEDs_Right, NUM_LEDS_PER_PANEL, 10);
    fadeToBlackBy(DJLEDs_Middle, NUM_LEDS_PER_PANEL, 10);
    fadeToBlackBy(DJLEDs_Left, NUM_LEDS_PER_PANEL, 10);

    // Add random twinkles
    for (int panel = 0; panel < 3; panel++) {
        CRGB* leds = getLEDArray(panel);

        if (random8() < 50) {
            int pos = random8(NUM_LEDS_PER_PANEL);
            // Random color with mostly white/blue tones
            uint8_t hue = random8() < 128 ? random8(140, 180) : random8(); // 50% blue-ish
            leds[pos] = CHSV(hue, random8(100, 255), 255);
        }

        // Occasionally add a bright white star
        if (random8() < 20) {
            int pos = random8(NUM_LEDS_PER_PANEL);
            leds[pos] = CRGB::White;
        }
    }
}

void initializePatterns() {
    // Pattern list is already initialized
}