#include "patterns_mouth.h"
#include "audio_processor.h"
#include "helpers.h"

// Helper function to determine if LED is on left or right side
bool isLeftSide(int row, int ledInRow) {
    int halfLeds = mouthRowLeds[row] / 2;
    return ledInRow < halfLeds;
}

// Get color based on split mode
CRGB getMouthColor(int row, int ledInRow) {
    CRGB color1 = getColor(mouthColorIndex);
    CRGB color2 = getColor(mouthColorIndex2);
    
    switch (mouthSplitMode) {
        case MOUTH_SPLIT_OFF:
            return color1;
            
        case MOUTH_SPLIT_LEFT_RIGHT:
            return isLeftSide(row, ledInRow) ? color1 : color2;
            
        case MOUTH_SPLIT_ALTERNATING:
            return (row % 2 == 0) ? color1 : color2;
            
        case MOUTH_SPLIT_CENTER_OUT:
            {
                int halfLeds = mouthRowLeds[row] / 2;
                int distance = abs(ledInRow - halfLeds);
                float ratio = (float)distance / halfLeds;
                return blend(color1, color2, ratio * 255);
            }
            
        case MOUTH_SPLIT_TOP_BOTTOM:
            return (row < MOUTH_ROWS / 2) ? color1 : color2;
            
        default:
            return color1;
    }
}

// Fixed brightness compensation for diffusor effect
CRGB adjustMouthBrightness(CRGB color, int row, int ledInRow) {
    // Boundary check
    if (row < 0 || row >= MOUTH_ROWS) return color;
    if (ledInRow < 0 || ledInRow >= mouthRowLeds[row]) return color;
    
    CRGB adjustedColor = color;
    
    // First apply mouth brightness (global mouth setting)
    adjustedColor.r = (adjustedColor.r * mouthBrightness) / 255;
    adjustedColor.g = (adjustedColor.g * mouthBrightness) / 255;
    adjustedColor.b = (adjustedColor.b * mouthBrightness) / 255;
    
    // Then apply boost factor based on LED position
    uint16_t boostFactor = 100; // Default 100% (no change)
    
    // For rows 0-7 (first 8 rows with 8 LEDs each)
    if (row < 8 && mouthRowLeds[row] == 8) {
        // Physical LEDs 1 and 8 (index 0 and 7) are outer LEDs
        if (ledInRow == 0 || ledInRow == 7) {
            boostFactor = mouthOuterBoost;
        } 
        // Physical LEDs 2-7 (index 1-6) are inner LEDs
        else {
            boostFactor = mouthInnerBoost;
        }
    } 
    // For rows 8-11 (last 4 rows), use inner boost for all
    else if (row >= 8) {
        boostFactor = mouthInnerBoost;
    }
    
    // Apply boost factor
    adjustedColor.r = min(255, (adjustedColor.r * boostFactor) / 100);
    adjustedColor.g = min(255, (adjustedColor.g * boostFactor) / 100);
    adjustedColor.b = min(255, (adjustedColor.b * boostFactor) / 100);
    
    return adjustedColor;
}

void updateMouth() {
    switch (mouthPattern) {
        case 0: mouthOff(); break;
        case 1: mouthTalk(); break;
        case 2: mouthSmile(); break;
        case 3: mouthAudioReactive(); break;
        case 4: mouthRainbow(); break;
        case 5: mouthDebug(); break;
        case 6: mouthWave(); break;
        case 7: mouthPulse(); break;
        case 8: mouthSparkle(); break;
        case 9: mouthScanline(); break;
        case 10: mouthFirework(); break;
        case 11: mouthAudioVU(); break;
        case 12: mouthMatrix(); break;
        case 13: mouthHeartbeat(); break;
        case 14: mouthSpectrum(); break;
    }
}

void mouthOff() {
    fadeToBlackBy(DJLEDs_Mouth, NUM_MOUTH_LEDS, 10);
}

void mouthTalk() {
    static uint8_t talkFrame = 0;
    static unsigned long lastTalkUpdate = 0;
    
    uint16_t talkInterval = map(talkSpeed, 1, 10, 500, 50);
    
    if (millis() - lastTalkUpdate > talkInterval) {
        lastTalkUpdate = millis();
        talkFrame = (talkFrame + 1) % 4;
        
        // Clear mouth
        fill_solid(DJLEDs_Mouth, NUM_MOUTH_LEDS, CRGB::Black);
        
        // Animate mouth opening/closing - using all rows
        switch (talkFrame) {
            case 0: // Closed
                for (int i = 0; i < mouthRowLeds[5]; i++) {
                    CRGB color = getMouthColor(5, i);
                    DJLEDs_Mouth[mouthRowStart[5] + i] = adjustMouthBrightness(color, 5, i);
                }
                for (int i = 0; i < mouthRowLeds[6]; i++) {
                    CRGB color = getMouthColor(6, i);
                    DJLEDs_Mouth[mouthRowStart[6] + i] = adjustMouthBrightness(color, 6, i);
                }
                break;
            case 1: // Slightly open
                for (int row = 3; row <= 8; row++) {
                    for (int i = 0; i < mouthRowLeds[row]; i++) {
                        CRGB color = getMouthColor(row, i);
                        DJLEDs_Mouth[mouthRowStart[row] + i] = adjustMouthBrightness(color, row, i);
                    }
                }
                break;
            case 2: // Open
                for (int row = 1; row <= 10; row++) {
                    for (int i = 0; i < mouthRowLeds[row]; i++) {
                        CRGB color = getMouthColor(row, i);
                        DJLEDs_Mouth[mouthRowStart[row] + i] = adjustMouthBrightness(color, row, i);
                    }
                }
                break;
            case 3: // Wide open - all rows
                for (int row = 0; row < MOUTH_ROWS; row++) {
                    for (int i = 0; i < mouthRowLeds[row]; i++) {
                        CRGB color = getMouthColor(row, i);
                        DJLEDs_Mouth[mouthRowStart[row] + i] = adjustMouthBrightness(color, row, i);
                    }
                }
                break;
        }
    }
}

void mouthSmile() {
    fill_solid(DJLEDs_Mouth, NUM_MOUTH_LEDS, CRGB::Black);
    
    // Draw smile shape
    int startRow = 6 - (smileWidth / 2);
    int endRow = 6 + (smileWidth / 2);
    
    for (int row = startRow; row <= endRow; row++) {
        if (row >= 0 && row < MOUTH_ROWS) {
            // Calculate smile curve
            int offset = abs(row - 6);
            int startLed = offset;
            int endLed = mouthRowLeds[row] - offset;
            
            for (int i = startLed; i < endLed; i++) {
                if (i >= 0 && i < mouthRowLeds[row]) {
                    CRGB color = getMouthColor(row, i);
                    DJLEDs_Mouth[mouthRowStart[row] + i] = adjustMouthBrightness(color, row, i);
                }
            }
        }
    }
}

void mouthAudioReactive() {
    int audio = audioProcessor.getLevel();
    
    // Check if audio should affect mouth
    if (audioMode == AUDIO_OFF || (audioMode != AUDIO_MOUTH_ONLY && audioMode != AUDIO_ALL)) {
        fadeToBlackBy(DJLEDs_Mouth, NUM_MOUTH_LEDS, 20);
        return;
    }
    
    // Clear mouth
    fadeToBlackBy(DJLEDs_Mouth, NUM_MOUTH_LEDS, 20);
    
    // Calculate how many rows to light based on audio
    int activeRows = map(audio, 0, audioThreshold, 0, MOUTH_ROWS);
    activeRows = constrain(activeRows, 0, MOUTH_ROWS);
    
    // Light rows from center outward
    int centerRow = 5;
    for (int i = 0; i < activeRows; i++) {
        int rowUp = centerRow - (i / 2);
        int rowDown = centerRow + (i / 2) + (i % 2);
        
        if (rowUp >= 0 && rowUp < MOUTH_ROWS) {
            for (int led = 0; led < mouthRowLeds[rowUp]; led++) {
                CRGB audioColor = getMouthColor(rowUp, led);
                audioColor = blend(audioColor, CRGB::White, map(audio, 0, audioThreshold, 0, 128));
                DJLEDs_Mouth[mouthRowStart[rowUp] + led] = adjustMouthBrightness(audioColor, rowUp, led);
            }
        }
        
        if (rowDown >= 0 && rowDown < MOUTH_ROWS && rowDown != rowUp) {
            for (int led = 0; led < mouthRowLeds[rowDown]; led++) {
                CRGB audioColor = getMouthColor(rowDown, led);
                audioColor = blend(audioColor, CRGB::White, map(audio, 0, audioThreshold, 0, 128));
                DJLEDs_Mouth[mouthRowStart[rowDown] + led] = adjustMouthBrightness(audioColor, rowDown, led);
            }
        }
    }
    
    // Add beat flash
    if (audioProcessor.isBeatDetected()) {
        for (int row = 0; row < MOUTH_ROWS; row++) {
            for (int i = 0; i < mouthRowLeds[row]; i++) {
                DJLEDs_Mouth[mouthRowStart[row] + i] = blend(DJLEDs_Mouth[mouthRowStart[row] + i], CRGB::White, 128);
            }
        }
    }
}

void mouthRainbow() {
    static uint8_t rainbowOffset = 0;
    
    for (int row = 0; row < MOUTH_ROWS; row++) {
        for (int i = 0; i < mouthRowLeds[row]; i++) {
            uint8_t hue = rainbowOffset + (row * 255 / MOUTH_ROWS) + (i * 255 / mouthRowLeds[row]);
            // Create base rainbow color at full brightness
            CRGB rowColor = CHSV(hue, 255, 255);
            
            // Apply split mode coloring on top of rainbow if enabled
            if (mouthSplitMode != MOUTH_SPLIT_OFF) {
                CRGB splitColor = getMouthColor(row, i);
                rowColor = blend(rowColor, splitColor, 128);
            }
            
            // Always apply brightness adjustment
            DJLEDs_Mouth[mouthRowStart[row] + i] = adjustMouthBrightness(rowColor, row, i);
        }
    }
    
    EVERY_N_MILLISECONDS(20) {
        rainbowOffset++;
    }
}

void mouthWave() {
    static uint8_t wavePosition = 0;
    static unsigned long lastWaveUpdate = 0;
    
    uint16_t waveInterval = map(waveSpeed, 1, 10, 200, 20);
    
    if (millis() - lastWaveUpdate > waveInterval) {
        lastWaveUpdate = millis();
        wavePosition++;
    }
    
    fadeToBlackBy(DJLEDs_Mouth, NUM_MOUTH_LEDS, 20);
    
    for (int row = 0; row < MOUTH_ROWS; row++) {
        uint8_t waveHeight = beatsin8(15 + row, 0, mouthRowLeds[row], 0, wavePosition);
        
        for (int i = 0; i < mouthRowLeds[row]; i++) {
            if (abs(i - waveHeight) < 2) {
                CRGB color = getMouthColor(row, i);
                uint8_t brightness = 255 - (abs(i - waveHeight) * 100);
                color.fadeToBlackBy(255 - brightness);
                DJLEDs_Mouth[mouthRowStart[row] + i] = adjustMouthBrightness(color, row, i);
            }
        }
    }
}

void mouthPulse() {
    static uint8_t pulsePhase = 0;
    static unsigned long lastPulseUpdate = 0;
    
    uint16_t pulseInterval = map(pulseSpeed, 1, 10, 100, 10);
    
    if (millis() - lastPulseUpdate > pulseInterval) {
        lastPulseUpdate = millis();
        pulsePhase += 5;
    }
    
    uint8_t pulseBright = beatsin8(30, 20, 255, 0, pulsePhase);
    
    for (int row = 0; row < MOUTH_ROWS; row++) {
        for (int i = 0; i < mouthRowLeds[row]; i++) {
            CRGB color = getMouthColor(row, i);
            
            // Add distance-based fade for center-out effect
            if (mouthSplitMode == MOUTH_SPLIT_CENTER_OUT) {
                int halfLeds = mouthRowLeds[row] / 2;
                int distance = abs(i - halfLeds);
                uint8_t distanceFade = map(distance, 0, halfLeds, 0, 100);
                uint8_t adjustedBright = pulseBright > distanceFade ? pulseBright - distanceFade : 0;
                color.fadeToBlackBy(255 - adjustedBright);
            } else {
                color.fadeToBlackBy(255 - pulseBright);
            }
            
            DJLEDs_Mouth[mouthRowStart[row] + i] = adjustMouthBrightness(color, row, i);
        }
    }
}

void mouthSparkle() {
    fadeToBlackBy(DJLEDs_Mouth, NUM_MOUTH_LEDS, 10);
    
    // Sparkle density based on speed
    uint8_t sparkleCount = map(effectSpeed, 1, 255, 2, 10);
    
    // Add random sparkles
    for (int i = 0; i < sparkleCount; i++) {
        int row = random8(MOUTH_ROWS);
        int led = random8(mouthRowLeds[row]);
        
        CRGB color = getMouthColor(row, led);
        color = blend(color, CRGB::White, random8(128, 255));
        
        DJLEDs_Mouth[mouthRowStart[row] + led] = adjustMouthBrightness(color, row, led);
    }
}

void mouthScanline() {
    static uint8_t scanRow = 0;
    static unsigned long lastScanUpdate = 0;
    static bool scanDirection = true;
    
    uint16_t scanInterval = map(effectSpeed, 1, 255, 200, 20);
    
    fadeToBlackBy(DJLEDs_Mouth, NUM_MOUTH_LEDS, 30);
    
    if (millis() - lastScanUpdate > scanInterval) {
        lastScanUpdate = millis();
        
        if (scanDirection) {
            scanRow++;
            if (scanRow >= MOUTH_ROWS - 1) scanDirection = false;
        } else {
            scanRow--;
            if (scanRow == 0) scanDirection = true;
        }
    }
    
    // Draw scanline with fade
    for (int r = max(0, scanRow - 2); r <= min(MOUTH_ROWS - 1, scanRow + 2); r++) {
        uint8_t brightness = 255 - (abs(r - scanRow) * 80);
        
        for (int i = 0; i < mouthRowLeds[r]; i++) {
            CRGB color = getMouthColor(r, i);
            color.fadeToBlackBy(255 - brightness);
            DJLEDs_Mouth[mouthRowStart[r] + i] |= adjustMouthBrightness(color, r, i);
        }
    }
}

void mouthFirework() {
    static unsigned long lastFirework = 0;
    static uint8_t fireworkRow = 0;
    static uint8_t fireworkLed = 0;
    static bool exploding = false;
    static uint8_t explosionRadius = 0;
    static CRGB explosionColor = CRGB::White;
    
    fadeToBlackBy(DJLEDs_Mouth, NUM_MOUTH_LEDS, 20);
    
    uint16_t fireworkInterval = map(effectSpeed, 1, 255, 2000, 500);
    
    if (!exploding && millis() - lastFirework > fireworkInterval) {
        lastFirework = millis();
        fireworkRow = random8(2, MOUTH_ROWS - 2);
        fireworkLed = random8(2, mouthRowLeds[fireworkRow] - 2);
        exploding = true;
        explosionRadius = 0;
        
        // Random explosion color
        if (random8() < 128) {
            explosionColor = getMouthColor(fireworkRow, fireworkLed);
        } else {
            explosionColor = CHSV(random8(), 255, 255);
        }
    }
    
    if (exploding) {
        explosionRadius++;
        
        // Draw explosion
        for (int r = max(0, fireworkRow - explosionRadius); r <= min(MOUTH_ROWS - 1, fireworkRow + explosionRadius); r++) {
            for (int i = 0; i < mouthRowLeds[r]; i++) {
                int distance = abs(r - fireworkRow) + abs(i - fireworkLed);
                if (distance <= explosionRadius && distance >= explosionRadius - 1) {
                    CRGB color = explosionColor;
                    if (random8() < 200) color = CRGB::White;
                    uint8_t fade = map(explosionRadius, 0, 5, 0, 200);
                    color.fadeToBlackBy(fade);
                    DJLEDs_Mouth[mouthRowStart[r] + i] |= adjustMouthBrightness(color, r, i);
                }
            }
        }
        
        if (explosionRadius > 5) {
            exploding = false;
        }
    }
}

void mouthAudioVU() {
    int audio = audioProcessor.getLevel();
    float average = audioProcessor.getAverageLevel();

    // Check if audio should affect mouth
    if (audioMode == AUDIO_OFF || (audioMode != AUDIO_MOUTH_ONLY && audioMode != AUDIO_ALL)) {
        fadeToBlackBy(DJLEDs_Mouth, NUM_MOUTH_LEDS, 20);
        return;
    }
    
    fadeToBlackBy(DJLEDs_Mouth, NUM_MOUTH_LEDS, 30);
    
    // Calculate VU levels for left and right
    int vuLevel = map(average, 0, audioThreshold, 0, mouthRowLeds[0]);
    int peakLevel = map(audio, 0, audioThreshold, 0, mouthRowLeds[0]);
    
    // Draw VU meter on each row
    for (int row = 0; row < MOUTH_ROWS; row++) {
        for (int i = 0; i < mouthRowLeds[row]; i++) {
            CRGB color = CRGB::Black;
            
            if (mouthSplitMode == MOUTH_SPLIT_LEFT_RIGHT) {
                // Stereo VU - mirror left and right
                int halfLeds = mouthRowLeds[row] / 2;
                if (isLeftSide(row, i)) {
                    int ledPos = halfLeds - i - 1;
                    if (ledPos < vuLevel) {
                        color = (ledPos < halfLeds/3) ? CRGB::Green : 
                               (ledPos < (halfLeds*2)/3) ? CRGB::Yellow : CRGB::Red;
                    }
                } else {
                    int ledPos = i - halfLeds;
                    if (ledPos < vuLevel) {
                        color = (ledPos < halfLeds/3) ? CRGB::Green : 
                               (ledPos < (halfLeds*2)/3) ? CRGB::Yellow : CRGB::Red;
                    }
                }
               
                // Add peak indicator
                if ((i == halfLeds - peakLevel - 1) || (i == halfLeds + peakLevel)) {
                    color = CRGB::White;
                }
            } else {
                // Mono VU - full width
                if (i < vuLevel) {
                    color = (i < mouthRowLeds[row]/3) ? CRGB::Green : 
                           (i < (mouthRowLeds[row]*2)/3) ? CRGB::Yellow : CRGB::Red;
                }
                if (i == peakLevel) {
                    color = CRGB::White;
                }
            }
           
            if (color != CRGB(0,0,0)) {
                DJLEDs_Mouth[mouthRowStart[row] + i] = adjustMouthBrightness(color, row, i);
            }
        }
    }
}

void mouthMatrix() {
   static uint8_t dropPos[MOUTH_ROWS];
   static uint8_t dropSpeed[MOUTH_ROWS];
   static bool dropActive[MOUTH_ROWS];
   static uint8_t dropBrightness[MOUTH_ROWS];
   
   fadeToBlackBy(DJLEDs_Mouth, NUM_MOUTH_LEDS, 30);
   
   for (int row = 0; row < MOUTH_ROWS; row++) {
       // Start new drop
       if (!dropActive[row] && random8() < 30) {
           dropActive[row] = true;
           dropPos[row] = 0;
           dropSpeed[row] = random8(1, 4);
           dropBrightness[row] = 255;
       }
       
       // Update active drops
       if (dropActive[row]) {
           // Draw the drop
           if (dropPos[row] < mouthRowLeds[row]) {
               CRGB color = getColor(matrixColorIndex); // Matrix Green
               color.fadeToBlackBy(255 - dropBrightness[row]);
               DJLEDs_Mouth[mouthRowStart[row] + dropPos[row]] = adjustMouthBrightness(color, row, dropPos[row]);
               
               // Trail
               for (int i = 1; i < 3; i++) {
                   if (dropPos[row] - i >= 0) {
                       CRGB trailColor = color;
                       trailColor.fadeToBlackBy(i * 80);
                       DJLEDs_Mouth[mouthRowStart[row] + dropPos[row] - i] |= adjustMouthBrightness(trailColor, row, dropPos[row] - i);
                   }
               }
           }
           
           // Move drop
           static unsigned long lastMatrixMove = 0;
           if (millis() - lastMatrixMove > (50 / dropSpeed[row])) {
               lastMatrixMove = millis();
               dropPos[row]++;
               
               // Fade brightness
               if (dropBrightness[row] > 20) {
                   dropBrightness[row] -= 20;
               }
           }
           
           // Check if drop finished
           if (dropPos[row] >= mouthRowLeds[row] + 3) {
               dropActive[row] = false;
           }
       }
   }
}

void mouthHeartbeat() {
   static uint8_t heartPhase = 0;
   static unsigned long lastBeat = 0;
   static bool beatState = false;
   static uint8_t beatBrightness = 0;
   
   // Heartbeat timing - lub-dub pattern
   unsigned long beatTiming[] = {80, 120, 80, 600}; // lub-dub-pause
   static uint8_t beatIndex = 0;
   
   if (millis() - lastBeat > beatTiming[beatIndex]) {
       lastBeat = millis();
       beatIndex = (beatIndex + 1) % 4;
       
       if (beatIndex == 0 || beatIndex == 2) {
           beatBrightness = 255; // Full brightness on beat
       }
   }
   
   // Fade out
   if (beatBrightness > 5) {
       beatBrightness -= 5;
   }
   
   // Clear mouth
   fill_solid(DJLEDs_Mouth, NUM_MOUTH_LEDS, CRGB::Black);
   
   // Draw heart shape with better definition
   for (int row = 0; row < MOUTH_ROWS; row++) {
       for (int i = 0; i < mouthRowLeds[row]; i++) {
           bool inHeart = false;
           
           // Heart shape definition by row
           switch(row) {
               case 0: // Top row
                   inHeart = (i >= 2 && i <= 5);
                   break;
               case 1:
                   inHeart = (i >= 1 && i <= 2) || (i >= 5 && i <= 6);
                   break;
               case 2:
                   inHeart = (i >= 0 && i <= 3) || (i >= 4 && i <= 7);
                   break;
               case 3:
                   inHeart = (i >= 0 && i <= 7);
                   break;
               case 4:
                   inHeart = (i >= 0 && i <= 7);
                   break;
               case 5:
                   inHeart = (i >= 1 && i <= 6);
                   break;
               case 6:
                   inHeart = (i >= 1 && i <= 6);
                   break;
               case 7:
                   inHeart = (i >= 2 && i <= 5);
                   break;
               case 8:
                   inHeart = (i >= 2 && i <= 3);
                   break;
               case 9:
                   inHeart = (i >= 1 && i <= 2);
                   break;
               case 10:
                   inHeart = (i == 1 || i == 2);
                   break;
               case 11:
                   inHeart = (i == 0 || i == 1);
                   break;
           }
           
           if (inHeart) {
               CRGB color = getColor(0); // Red
               color.fadeToBlackBy(255 - beatBrightness);
               DJLEDs_Mouth[mouthRowStart[row] + i] = adjustMouthBrightness(color, row, i);
           }
       }
   }
}

void mouthSpectrum() {
   // Audio spectrum visualization
   float bass = audioProcessor.getBassLevel() / 100.0;
   float mid = audioProcessor.getMidLevel() / 100.0;
   float treble = audioProcessor.getTrebleLevel() / 100.0;
   
   // Clear mouth with fade
   fadeToBlackBy(DJLEDs_Mouth, NUM_MOUTH_LEDS, 30);
   
   // Draw spectrum bars
   for (int row = 0; row < MOUTH_ROWS; row++) {
       // Calculate which frequency band this row represents
       float freq = 0;
       CRGB color;
       
       if (row < 4) {
           // Bass rows (bottom)
           freq = bass;
           color = CRGB(255, 0, 0); // Red for bass
       } else if (row < 8) {
           // Mid rows (middle)
           freq = mid;
           color = CRGB(255, 255, 0); // Yellow for mids
       } else {
           // Treble rows (top)
           freq = treble;
           color = CRGB(0, 100, 255); // Blue for treble
       }
       
       // Apply beat modulation
       if (audioProcessor.isBeatDetected() && row < 4) {
           freq = 1.0; // Full bass on beat
           color = CRGB::White;
       }
       
       // Number of LEDs to light based on frequency level
       int numLeds = (int)(mouthRowLeds[row] * freq);
       
       // Draw the bar centered
       for (int i = 0; i < numLeds; i++) {
           int ledIndex = (mouthRowLeds[row] - numLeds) / 2 + i;
           if (ledIndex >= 0 && ledIndex < mouthRowLeds[row]) {
               // Add gradient effect
               CRGB ledColor = color;
               uint8_t fade = map(i, 0, numLeds - 1, 50, 0);
               ledColor.fadeToBlackBy(fade);
               
               DJLEDs_Mouth[mouthRowStart[row] + ledIndex] = adjustMouthBrightness(ledColor, row, ledIndex);
           }
       }
       
       // Add peak dots
       if (freq > 0.8) {
           int peakLed = mouthRowLeds[row] / 2;
           DJLEDs_Mouth[mouthRowStart[row] + peakLed] = adjustMouthBrightness(CRGB::White, row, peakLed);
       }
   }
}

void mouthDebug() {
   static uint8_t debugMode = 0;
   static unsigned long lastDebugTime = 0;
   static uint8_t testRow = 0;
   static uint8_t testLed = 0;
   
   // Clear all
   fill_solid(DJLEDs_Mouth, NUM_MOUTH_LEDS, CRGB::Black);
   
   // Switch debug mode every 3 seconds
   if (millis() - lastDebugTime > 3000) {
       lastDebugTime = millis();
       debugMode = (debugMode + 1) % 6;
       testRow = 0;
       testLed = 0;
       
       Serial.print(F("Mouth Debug Mode: "));
       switch(debugMode) {
           case 0: Serial.println(F("Row test")); break;
           case 1: Serial.println(F("LED mapping test")); break;
           case 2: Serial.println(F("Brightness compensation test")); break;
           case 3: Serial.println(F("Split mode test")); break;
           case 4: Serial.println(F("Full white test")); break;
           case 5: Serial.println(F("Address test")); break;
       }
   }
   
   CRGB testColor1 = CRGB(100, 0, 0);   // Red for primary
   CRGB testColor2 = CRGB(0, 0, 100);   // Blue for secondary
   
   switch(debugMode) {
       case 0: // Row test - light one row at a time
           {
               static unsigned long lastRowTime = 0;
               if (millis() - lastRowTime > 500) {
                   lastRowTime = millis();
                   testRow = (testRow + 1) % MOUTH_ROWS;
               }
               
               for (int i = 0; i < mouthRowLeds[testRow]; i++) {
                   DJLEDs_Mouth[mouthRowStart[testRow] + i] = adjustMouthBrightness(testColor1, testRow, i);
               }
               
               // Show row info
               if (millis() - lastRowTime < 100) {
                   Serial.print(F("Row "));
                   Serial.print(testRow);
                   Serial.print(F(": "));
                   Serial.print(mouthRowLeds[testRow]);
                   Serial.print(F(" LEDs, Start: "));
                   Serial.println(mouthRowStart[testRow]);
               }
           }
           break;
           
       case 1: // LED mapping - sweep through all LEDs
           {
               static unsigned long lastLedTime = 0;
               static int currentLed = 0;
               
               if (millis() - lastLedTime > 50) {
                   lastLedTime = millis();
                   currentLed = (currentLed + 1) % NUM_MOUTH_LEDS;
               }
               
               // Light current LED white
               DJLEDs_Mouth[currentLed] = CRGB::White;
               
               // Find which row this LED belongs to
               for (int row = 0; row < MOUTH_ROWS; row++) {
                   if (currentLed >= mouthRowStart[row] && 
                       currentLed < mouthRowStart[row] + mouthRowLeds[row]) {
                       // Light rest of row dim
                       for (int i = 0; i < mouthRowLeds[row]; i++) {
                           if (mouthRowStart[row] + i != currentLed) {
                               DJLEDs_Mouth[mouthRowStart[row] + i] = CRGB(20, 20, 20);
                           }
                       }
                       break;
                   }
               }
           }
           break;
           
       case 2: // Brightness compensation test
           for (int row = 0; row < MOUTH_ROWS; row++) {
               for (int i = 0; i < mouthRowLeds[row]; i++) {
                   // Outer LEDs red, inner LEDs blue
                   CRGB color;
                   if (row < 8 && (i == 0 || i == 7)) {
                       color = testColor1; // Red for outer
                   } else {
                       color = testColor2; // Blue for inner
                   }
                   DJLEDs_Mouth[mouthRowStart[row] + i] = adjustMouthBrightness(color, row, i);
               }
           }
           break;
           
       case 3: // Split mode visualization
           for (int row = 0; row < MOUTH_ROWS; row++) {
               for (int i = 0; i < mouthRowLeds[row]; i++) {
                   CRGB color = getMouthColor(row, i);
                   DJLEDs_Mouth[mouthRowStart[row] + i] = adjustMouthBrightness(color, row, i);
               }
           }
           break;
           
       case 4: // Full white test
           for (int row = 0; row < MOUTH_ROWS; row++) {
               for (int i = 0; i < mouthRowLeds[row]; i++) {
                   CRGB color = CRGB(100, 100, 100); // Gray
                   DJLEDs_Mouth[mouthRowStart[row] + i] = adjustMouthBrightness(color, row, i);
               }
           }
           break;
           
       case 5: // Address test - show LED addresses
           {
               // Light LEDs at specific addresses to verify mapping
               int testAddresses[] = {0, 7, 8, 15, 16, 23, 24, 31, 32, 39, 40, 47, 48, 55, 56, 63, 64, 69, 70, 73, 74, 77, 78, 79};
               for (int i = 0; i < 24; i++) {
                   if (testAddresses[i] < NUM_MOUTH_LEDS) {
                       DJLEDs_Mouth[testAddresses[i]] = CRGB::Green;
                   }
               }
           }
           break;
   }
}