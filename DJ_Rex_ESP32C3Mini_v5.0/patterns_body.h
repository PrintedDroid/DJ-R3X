#ifndef PATTERNS_BODY_H
#define PATTERNS_BODY_H

#include "config.h"
#include "globals.h"

// Pattern functions
void LEDsOff();
void RandomBlocks();
void SolidColor();
void ShortCircuit();
void ConfettiRedWhite();
void rainbow();
void rainbowWithGlitter();
void confetti();
void juggle();
void audioSync();
void SolidFlash();
void knightRider();
void breathing();
void matrixRain();
void strobePattern();
void audioVUMeter();
void CustomBlockSequence();

// v5.0: New patterns
void plasmaPattern();
void firePattern();
void twinklePattern();

// Helper for rainbow
void addGlitter(fract8 chanceOfGlitter);

// Initialize pattern list
void initializePatterns();

#endif