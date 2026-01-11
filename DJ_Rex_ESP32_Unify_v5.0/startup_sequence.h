// startup_sequence.h - v5.0 Animated Boot Sequence
#ifndef STARTUP_SEQUENCE_H
#define STARTUP_SEQUENCE_H

#include "config.h"
#include "globals.h"

class StartupSequence {
public:
    void begin();
    void run();
    bool isComplete();

private:
    enum Phase {
        PHASE_INIT,
        PHASE_LED_TEST,
        PHASE_EYES_ON,
        PHASE_MOUTH_ON,
        PHASE_BODY_SWEEP,
        PHASE_FLASH,
        PHASE_COMPLETE
    };

    Phase currentPhase = PHASE_INIT;
    unsigned long phaseStartTime = 0;
    uint8_t sweepPosition = 0;
    bool complete = false;

    void runPhase();
    void nextPhase();
};

extern StartupSequence startupSequence;

#endif
