#include "patterns_body.h"
#include "helpers.h"
#include "audio_processor.h"

// Pattern list definition
SimplePatternList gPatterns = {
    LEDsOff,              // 0
    RandomBlocks,         // 1
    SolidColor,           // 2
    ShortCircuit,         // 3
    ConfettiRedWhite,     // 4
    rainbow,              // 5
    rainbowWithGlitter,   // 6
    confetti,             // 7
    juggle,               // 8
    audioSync,            // 9
    SolidFlash,           // 10
    knightRider,          // 11
    breathing,            // 12
    matrixRain,           // 13
    strobePattern,        // 14
    audioVUMeter,         // 15
    CustomBlockSequence,  // 16
    plasmaPattern,        // 17
    firePattern,          // 18
    twinklePattern        // 19
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
        // Static mode
        CRGB color = applyBodyBrightness(getColor(solidColorIndex));
        fill_solid(DJLEDs_Right, NUM_LEDS_PER_PANEL, color);
        fill_solid(DJLEDs_Middle, NUM_LEDS_PER_PANEL, color);
        fill_solid(DJLEDs_Left, NUM_LEDS_PER_PANEL, color);
    } else {
        // Blink mode
        CRGB selectedColor = applyBodyBrightness(getColor(solidColorIndex));
        
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
        CRGB sparkColor = applyBodyBrightness(getColor(shortColorIndex));
        
        if (random8() < SHORT_CIRCUIT_SPARK_CHANCE) {
            DJLEDs_Right[random16(NUM_LEDS_PER_PANEL)] += sparkColor;
        }
        if (random8() < SHORT_CIRCUIT_SPARK_CHANCE) {
            DJLEDs_Middle[random16(NUM_LEDS_PER_PANEL)] += sparkColor;
        }
        if (random8() < SHORT_CIRCUIT_SPARK_CHANCE) {
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

    CRGB color1 = applyBodyBrightness(getColor(confettiColor1));
    CRGB color2 = applyBodyBrightness(getColor(confettiColor2));
    
    if (random8() < CONFETTI_SPLIT_CHANCE) {
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
    
    // Apply body brightness
    for (int i = 0; i < NUM_LEDS_PER_PANEL; i++) {
        DJLEDs_Right[i] = applyBodyBrightness(DJLEDs_Right[i]);
        DJLEDs_Middle[i] = applyBodyBrightness(DJLEDs_Middle[i]);
        DJLEDs_Left[i] = applyBodyBrightness(DJLEDs_Left[i]);
    }
}

void rainbowWithGlitter() {
    rainbow();
    addGlitter(GLITTER_CHANCE);
}

void addGlitter(fract8 chanceOfGlitter) {
    if (random8() < chanceOfGlitter) {
        DJLEDs_Right[random16(NUM_LEDS_PER_PANEL)] += applyBodyBrightness(CRGB::White);
    }
    if (random8() < chanceOfGlitter) {
        DJLEDs_Middle[random16(NUM_LEDS_PER_PANEL)] += applyBodyBrightness(CRGB::White);
    }
    if (random8() < chanceOfGlitter) {
        DJLEDs_Left[random16(NUM_LEDS_PER_PANEL)] += applyBodyBrightness(CRGB::White);
    }
}

void confetti() {
    fadeToBlackBy(DJLEDs_Right, NUM_LEDS_PER_PANEL, fadeSpeed);
    fadeToBlackBy(DJLEDs_Middle, NUM_LEDS_PER_PANEL, fadeSpeed);
    fadeToBlackBy(DJLEDs_Left, NUM_LEDS_PER_PANEL, fadeSpeed);

    DJLEDs_Right[random16(NUM_LEDS_PER_PANEL)] += applyBodyBrightness(CHSV(gHue + random8(64), 200, 255));
    DJLEDs_Middle[random16(NUM_LEDS_PER_PANEL)] += applyBodyBrightness(CHSV(gHue + random8(64), 200, 255));
    DJLEDs_Left[random16(NUM_LEDS_PER_PANEL)] += applyBodyBrightness(CHSV(gHue + random8(64), 200, 255));
}

void juggle() {
    fadeToBlackBy(DJLEDs_Right, NUM_LEDS_PER_PANEL, JUGGLE_FADE_RATE);
    fadeToBlackBy(DJLEDs_Middle, NUM_LEDS_PER_PANEL, JUGGLE_FADE_RATE);
    fadeToBlackBy(DJLEDs_Left, NUM_LEDS_PER_PANEL, JUGGLE_FADE_RATE);

    byte dothue = 0;
    for (int i = 0; i < 8; i++) {
        CRGB color = applyBodyBrightness(CHSV(dothue, 200, 255));
        DJLEDs_Right[beatsin16(i + 7, 0, NUM_LEDS_PER_PANEL - 1)] |= color;
        DJLEDs_Middle[beatsin16(i + 7, 0, NUM_LEDS_PER_PANEL - 1)] |= color;
        DJLEDs_Left[beatsin16(i + 7, 0, NUM_LEDS_PER_PANEL - 1)] |= color;
        dothue += 32;
    }
}

void audioSync() {
    int audio = audioProcessor.getLevel();
    
    // Check audio mode for body panels
    if (audioMode == AUDIO_OFF || audioMode == AUDIO_MOUTH_ONLY) {
        // Clear panels and return
        fadeToBlackBy(DJLEDs_Right, NUM_LEDS_PER_PANEL, AUDIO_SYNC_FADE_RATE);
        fadeToBlackBy(DJLEDs_Middle, NUM_LEDS_PER_PANEL, AUDIO_SYNC_FADE_RATE);
        fadeToBlackBy(DJLEDs_Left, NUM_LEDS_PER_PANEL, AUDIO_SYNC_FADE_RATE);
        return;
    }
    
    int numLEDs = map(audio, 0, audioThreshold, 0, SIDE_LEDS_COUNT);
    numLEDs = constrain(numLEDs, 0, SIDE_LEDS_COUNT);
    
    fadeToBlackBy(DJLEDs_Right, NUM_LEDS_PER_PANEL, AUDIO_SYNC_FADE_RATE);
    fadeToBlackBy(DJLEDs_Middle, NUM_LEDS_PER_PANEL, AUDIO_SYNC_FADE_RATE);
    fadeToBlackBy(DJLEDs_Left, NUM_LEDS_PER_PANEL, AUDIO_SYNC_FADE_RATE);
    
    CRGB audioColor = applyBodyBrightness(CHSV(gHue, 255, 255));
    
    // Light up side LEDs
    for (int i = 0; i < numLEDs; i++) {
        DJLEDs_Right[i] = audioColor;
        DJLEDs_Middle[i] = audioColor;
        DJLEDs_Left[i] = audioColor;
    }
    
    // Add blocks if audio mode includes all body
    if (audioMode == AUDIO_BODY_ALL || audioMode == AUDIO_ALL) {
        if (audio > audioThreshold * AUDIO_MEDIUM_LEVEL_THRESHOLD) {
            for (int panel = 0; panel < 3; panel++) {
                if (audio > audioThreshold * 0.9) {
                    setBlock(panel, BLOCK3_START, audioColor);
                }
                if (audio > audioThreshold * AUDIO_HIGH_LEVEL_THRESHOLD) {
                    setBlock(panel, BLOCK2_START, audioColor);
                }
                setBlock(panel, BLOCK1_START, audioColor);
            }
        }
    }
    
    // Add sparkle on high levels
    if (audio > audioThreshold * AUDIO_HIGH_LEVEL_THRESHOLD) {
        for (int panel = 0; panel < 3; panel++) {
            CRGB* leds = getLEDArray(panel);
            if (random8() < AUDIO_SPARKLE_CHANCE) {
                leds[random8(SIDE_LEDS_COUNT)] += applyBodyBrightness(CRGB::White);
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
    
    CRGB displayColor = flashState ? applyBodyBrightness(getColor(flashColorIndex)) : CRGB::Black;
    
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
            if (knightPos >= SIDE_LEDS_COUNT - 1) {
                knightPos = SIDE_LEDS_COUNT - 1;
                knightDir = false;
            }
        } else {
            if (knightPos > 0) {
                knightPos--;
            } else {
                knightPos = 0;
                knightDir = true;
            }
        }
    }
    
    // Ensure knightPos is within valid range
    knightPos = constrain(knightPos, 0, SIDE_LEDS_COUNT - 1);
    
    CRGB color = applyBodyBrightness(getColor(knightColorIndex));
    DJLEDs_Right[knightPos] = color;
    DJLEDs_Middle[knightPos] = color;
    DJLEDs_Left[knightPos] = color;
    
    // Draw trail with bounds checking
    if (knightPos > 0) {
        CRGB trailColor = color;
        trailColor.fadeToBlackBy(KNIGHT_RIDER_FADE);
        DJLEDs_Right[knightPos - 1] = trailColor;
        DJLEDs_Middle[knightPos - 1] = trailColor;
        DJLEDs_Left[knightPos - 1] = trailColor;
    }
    if (knightPos < SIDE_LEDS_COUNT - 1) {
        CRGB trailColor = color;
        trailColor.fadeToBlackBy(KNIGHT_RIDER_FADE);
        DJLEDs_Right[knightPos + 1] = trailColor;
        DJLEDs_Middle[knightPos + 1] = trailColor;
        DJLEDs_Left[knightPos + 1] = trailColor;
    }
    
    // Extended trail for smoother effect
    if (knightPos > 1) {
        CRGB trailColor2 = color;
        trailColor2.fadeToBlackBy(min(255, KNIGHT_RIDER_FADE * 2));
        DJLEDs_Right[knightPos - 2] = trailColor2;
        DJLEDs_Middle[knightPos - 2] = trailColor2;
        DJLEDs_Left[knightPos - 2] = trailColor2;
    }
    if (knightPos < SIDE_LEDS_COUNT - 2) {
        CRGB trailColor2 = color;
        trailColor2.fadeToBlackBy(min(255, KNIGHT_RIDER_FADE * 2));
        DJLEDs_Right[knightPos + 2] = trailColor2;
        DJLEDs_Middle[knightPos + 2] = trailColor2;
        DJLEDs_Left[knightPos + 2] = trailColor2;
    }
    
    // Light up blocks in sequence
    static uint8_t blockSequence = 0;
    static unsigned long lastBlockTime = 0;
    
    if (millis() - lastBlockTime > knightInterval * 8) {
        lastBlockTime = millis();
        blockSequence = (blockSequence + 1) % 9;
        
        for (int panel = 0; panel < 3; panel++) {
            for (int block = 0; block < 3; block++) {
                uint8_t globalIndex = getGlobalBlockIndex(panel, block);
                if (globalIndex == blockSequence) {
                    uint8_t blockStarts[3] = {BLOCK1_START, BLOCK2_START, BLOCK3_START};
                    setBlock(panel, blockStarts[block], color);
                }
            }
        }
    }
}

void breathing() {
    static CRGB lastColor = CRGB::Black;
    uint16_t speed = map(effectSpeed, 1, 255, 10, 2);
    
    if (millis() - breathingMillis > speed) {
        breathingMillis = millis();
        if (breathingUp) {
            breathingBright += 2;
            if (breathingBright >= 255) { 
                breathingBright = 255; 
                breathingUp = false; 
            }
        } else {
            breathingBright -= 2;
            if (breathingBright <= 10) { 
                breathingBright = 10; 
                breathingUp = true; 
            }
        }
    }
    
    CRGB targetColor = applyBodyBrightness(getColor(breathingColorIndex));
    targetColor.fadeToBlackBy(255 - breathingBright);
    
    // Smooth transition between colors - using helper function
    CRGB displayColor = smoothColorTransition(lastColor, targetColor, colorFadeSteps);
    lastColor = displayColor;
    
    fill_solid(DJLEDs_Right, NUM_LEDS_PER_PANEL, displayColor);
    fill_solid(DJLEDs_Middle, NUM_LEDS_PER_PANEL, displayColor);
    fill_solid(DJLEDs_Left, NUM_LEDS_PER_PANEL, displayColor);
}

void matrixRain() {
    uint16_t speed = map(effectSpeed, 1, 255, 150, 20);
    
    if (millis() - matrixMillis > speed) {
        matrixMillis = millis();
        
        for (int panel = 0; panel < 3; panel++) {
            CRGB* leds = getLEDArray(panel);
            
            // Shift drops down with bounds checking
            for (int led = SIDE_LEDS_COUNT - 1; led > 0; led--) {
                if (led < SIDE_LEDS_COUNT) {
                    matrixDrops[panel][led] = matrixDrops[panel][led - 1];
                    matrixBright[panel][led] = matrixBright[panel][led - 1];
                }
            }
            
            // New drop at top
            if (random8() < MATRIX_DROP_CHANCE) {
                matrixDrops[panel][0] = 0;
                matrixBright[panel][0] = 255;
            } else {
                matrixDrops[panel][0] = 255;
                matrixBright[panel][0] = 0;
            }
            
            // Clear all LEDs first
            for (int i = 0; i < SIDE_LEDS_COUNT; i++) {
                leds[i] = CRGB::Black;
            }
            
            // Draw drops
            CRGB color = applyBodyBrightness(getColor(matrixColorIndex));
            for (int led = 0; led < SIDE_LEDS_COUNT; led++) {
                if (matrixDrops[panel][led] == 0) {
                    CRGB dropColor = color;
                    dropColor.fadeToBlackBy(255 - matrixBright[panel][led]);
                    leds[led] = dropColor;
                    
                    // Fade the drop
                    if (matrixBright[panel][led] > MATRIX_FADE_STEP) {
                        matrixBright[panel][led] -= MATRIX_FADE_STEP;
                    } else {
                        matrixDrops[panel][led] = 255;
                        matrixBright[panel][led] = 0;
                    }
                }
            }
            
            // Clear block LEDs
            for (int i = SIDE_LEDS_COUNT; i < NUM_LEDS_PER_PANEL; i++) {
                leds[i] = CRGB::Black;
            }
            
            // Random block glitches
            if (random8() < 30) {
                uint8_t blockStarts[3] = {BLOCK1_START, BLOCK2_START, BLOCK3_START};
                uint8_t randomBlock = random8(3);
                CRGB glitchColor = color;
                glitchColor.fadeToBlackBy(random8(100, 200));
                setBlock(panel, blockStarts[randomBlock], glitchColor);
            }
        }
    }
}

void strobePattern() {
    strobeInterval = map(effectSpeed, 1, 255, 300, 50);
    
    if (millis() - strobeMillis > strobeInterval) {
        strobeMillis = millis();
        strobeState = !strobeState;
    }
    
    CRGB color = strobeState ? applyBodyBrightness(getColor(strobeColorIndex)) : CRGB::Black;
    
    fill_solid(DJLEDs_Right, NUM_LEDS_PER_PANEL, color);
    fill_solid(DJLEDs_Middle, NUM_LEDS_PER_PANEL, color);
    fill_solid(DJLEDs_Left, NUM_LEDS_PER_PANEL, color);
}

void audioVUMeter() {
    int audio = audioProcessor.getLevel();
    float average = audioProcessor.getAverageLevel();
    
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
    
    int vuLevel = constrain(map(average, 0, audioThreshold, 0, SIDE_LEDS_COUNT), 0, SIDE_LEDS_COUNT);
    
    for (int panel = 0; panel < 3; panel++) {
        CRGB* leds = getLEDArray(panel);
        for (int i = 0; i < vuLevel; i++) {
            int ledIdx = SIDE_LEDS_COUNT - 1 - i;
            CRGB color = (i < SIDE_LEDS_COUNT/3) ? CRGB::Green : 
                         (i < (SIDE_LEDS_COUNT*2)/3) ? CRGB::Yellow : CRGB::Red;
            leds[ledIdx] = applyBodyBrightness(color);
        }
        
        // Peak indicators on blocks if mode allows
        if ((audioMode == AUDIO_BODY_ALL || audioMode == AUDIO_ALL) && average > audioThreshold * AUDIO_HIGH_LEVEL_THRESHOLD) {
            setBlock(panel, BLOCK1_START, applyBodyBrightness(CRGB::White));
            setBlock(panel, BLOCK2_START, applyBodyBrightness(CRGB::White));
            setBlock(panel, BLOCK3_START, applyBodyBrightness(CRGB::White));
        }
    }
    
    if (audio > audioThreshold * VU_METER_SPARKLE_THRESHOLD && random8() < VU_METER_SPARKLE_CHANCE) {
        getLEDArray(random8(3))[random8(SIDE_LEDS_COUNT)] += applyBodyBrightness(CRGB::White);
    }
}

void CustomBlockSequence() {
    // Define the block colors for the sequence
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
                    leds[i] = applyBodyBrightness(getColor(colorIndex));
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
                    CRGB blockColor = applyBodyBrightness(getColor(sequenceColors[globalBlockIndex]));
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

void plasmaPattern() {
    static uint16_t time = 0;
    time += map(effectSpeed, 1, 255, 1, 10);
    
    for (uint8_t panel = 0; panel < 3; panel++) {
        CRGB* leds = getLEDArray(panel);
        
        // Side LEDs plasma
        for (int i = 0; i < SIDE_LEDS_COUNT; i++) {
            uint8_t hue1 = sin8(i * 30 + time * 2);
            uint8_t hue2 = sin8(i * 50 - time * 3);
            uint8_t hue3 = sin8((i * 10) + time);
            
            uint8_t hue = (hue1 + hue2 + hue3) / 3;
            
            leds[i] = CHSV(hue, 255, 255);
            leds[i] = applyBodyBrightness(leds[i]);
        }
        
        // Blocks with different plasma parameters
        uint8_t blockStarts[3] = {BLOCK1_START, BLOCK2_START, BLOCK3_START};
        for (uint8_t block = 0; block < 3; block++) {
            uint8_t blockHue = sin8(block * 85 + time * 4) + sin8(panel * 100 - time * 3);
            CRGB blockColor = CHSV(blockHue, 200, 255);
            setBlock(panel, blockStarts[block], blockColor);
        }
    }
}

void firePattern() {
    // Heat array for each panel
    static uint8_t heat[3][NUM_LEDS_PER_PANEL];
    static uint8_t sparkHeight[3] = {0, 0, 0};
    
    for (uint8_t panel = 0; panel < 3; panel++) {
        CRGB* leds = getLEDArray(panel);
        
        // Cool down every cell a little
        for (int i = 0; i < NUM_LEDS_PER_PANEL; i++) {
            heat[panel][i] = qsub8(heat[panel][i], random8(0, ((55 * 10) / NUM_LEDS_PER_PANEL) + 2));
        }
        
        // Heat from each cell drifts 'up' and diffuses slightly
        // Side LEDs
        for (int k = SIDE_LEDS_COUNT - 1; k >= 2; k--) {
            heat[panel][k] = (heat[panel][k - 1] + heat[panel][k - 2] + heat[panel][k - 2]) / 3;
        }
        
        // Randomly ignite new 'sparks' near the bottom
        if (random8() < 120) {
            int y = random8(3);
            heat[panel][y] = qadd8(heat[panel][y], random8(160, 255));
            sparkHeight[panel] = y;
        }
        
        // Convert heat to LED colors for side LEDs
        for (int j = 0; j < SIDE_LEDS_COUNT; j++) {
            CRGB color = HeatColor(heat[panel][j]);
            leds[j] = applyBodyBrightness(color);
        }
        
        // Fire effect on blocks based on spark height
        uint8_t blockStarts[3] = {BLOCK1_START, BLOCK2_START, BLOCK3_START};
        for (int block = 0; block < 3; block++) {
            uint8_t blockHeat = 0;
            if (sparkHeight[panel] > 5 + block * 2) {
                blockHeat = heat[panel][sparkHeight[panel]] / (block + 1);
            }
            CRGB blockColor = HeatColor(blockHeat);
            setBlock(panel, blockStarts[block], blockColor);
        }
    }
}

void twinklePattern() {
    // Fade everything
    fadeToBlackBy(DJLEDs_Right, NUM_LEDS_PER_PANEL, 10);
    fadeToBlackBy(DJLEDs_Middle, NUM_LEDS_PER_PANEL, 10);
    fadeToBlackBy(DJLEDs_Left, NUM_LEDS_PER_PANEL, 10);
    
    // Twinkle speed based on effectSpeed
    uint8_t twinkleChance = map(effectSpeed, 1, 255, 10, 100);
    
    // Add random twinkles on side LEDs
    for (int i = 0; i < 3; i++) {
        if (random8() < twinkleChance) {
            uint8_t panel = random8(3);
            CRGB* leds = getLEDArray(panel);
            int pos = random8(SIDE_LEDS_COUNT);
            
            // Random color or white
            CRGB twinkleColor;
            if (random8() < 128) {
                twinkleColor = CRGB::White;
            } else {
                twinkleColor = CHSV(random8(), 255, 255);
            }
            
            leds[pos] = applyBodyBrightness(twinkleColor);
        }
    }
    
    // Occasional block twinkles
    if (random8() < twinkleChance / 2) {
        uint8_t panel = random8(3);
        uint8_t block = random8(3);
        uint8_t blockStarts[3] = {BLOCK1_START, BLOCK2_START, BLOCK3_START};
        
        CRGB blockColor = CHSV(random8(), random8(200, 255), 255);
        setBlock(panel, blockStarts[block], blockColor);
    }
}

void initializePatterns() {
    // Pattern list is already initialized statically
}