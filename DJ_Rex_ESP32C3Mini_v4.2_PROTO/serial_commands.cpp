#include "serial_commands.h"
#include "settings.h"
#include "helpers.h"
#include "pattern_manager.h"
#include "preset_manager.h"
#include "audio_processor.h"
#include "event_logger.h"
#include "eyes.h"  // NEW: For eye flicker settings

// Serial input buffer management - THREAD SAFE
static const unsigned long COMMAND_TIMEOUT_MS = 5000;
static const uint8_t MAX_COMMAND_BUFFER_SIZE = 80; // Reduced for safety
static unsigned long commandStartTime = 0;
static bool commandInProgress = false;
static uint32_t totalCommands = 0;

// Command processing state
static String lastCommand = "";
static unsigned long lastCommandTime = 0;
static uint8_t repeatCommandCount = 0;

// Buffer for safe string operations
static char tempBuffer[MAX_COMMAND_BUFFER_SIZE];

// Structured output helpers
void printSeparator() {
    Serial.println(F("========================================"));
}

void printSection(const char* title) {
    Serial.println();
    Serial.print(F(">>> "));
    Serial.print(title);
    Serial.println(F(" <<<"));
}

bool checkSerialCommand() {
    // Prevent concurrent command processing
    if (commandInProgress) {
        return false;
    }
    
    while (Serial.available()) {
        // Initialize command timing on first character
        if (inputString.length() == 0 && commandStartTime == 0) {
            commandStartTime = millis();
            commandInProgress = true;
        }
        
        // CRITICAL: Check buffer size BEFORE reading character
        if (inputString.length() >= MAX_COMMAND_BUFFER_SIZE - 2) {
            Serial.print(F("\n! Command buffer full ("));
            Serial.print(inputString.length());
            Serial.print(F("/"));
            Serial.print(MAX_COMMAND_BUFFER_SIZE);
            Serial.println(F(" chars)"));
            
            // Find end of command or force termination
            bool foundEnd = false;
            uint8_t extraChars = 0;
            while (Serial.available() && extraChars < 50) { // Max 50 extra chars
                char discardChar = Serial.read();
                extraChars++;
                if (discardChar == '\n' || discardChar == '\r') {
                    foundEnd = true;
                    break;
                }
            }
            
            if (foundEnd) {
                Serial.println(F("! Command truncated but processing"));
                stringComplete = true;
                return true;
            } else {
                Serial.println(F("! Command too long - discarded"));
                inputString = "";
                stringComplete = false;
                commandStartTime = 0;
                commandInProgress = false;
                failedCommands++;
                return false;
            }
        }
        
        char inChar = (char)Serial.read();
        
        // Handle command termination
        if (inChar == '\n') {
            stringComplete = true;
            commandInProgress = false;
            return true;
        } else if (inChar == '\r') {
            continue; // Ignore carriage returns
        }
        
        // Filter out control characters except printable ASCII
        if (inChar < 32 || inChar > 126) {
            Serial.print(F("! Filtered invalid char: "));
            Serial.println((int)inChar);
            continue;
        }
        
        // Safe to add character - we checked buffer size above
        inputString += inChar;
    }
    
    // Timeout handling
    if (commandStartTime > 0 && millis() - commandStartTime > COMMAND_TIMEOUT_MS) {
        Serial.println(F("\n! Command timeout - processing partial input"));
        if (inputString.length() > 0) {
            stringComplete = true;
            commandInProgress = false;
            return true;
        } else {
            Serial.println(F("! Empty command after timeout"));
            commandStartTime = 0;
            commandInProgress = false;
            failedCommands++;
            return false;
        }
    }
    
    return false;
}

void printHelp() {
    printSeparator();
    Serial.println(F("DJ Rex v4.2.0 - Complete Command Reference"));
    printSeparator();
    
    printSection("Body Pattern Commands");
    Serial.println(F("  s <0-19>           - Set pattern"));
    Serial.println(F("  next/prev          - Navigate patterns"));
    
    printSection("Eye Commands");
    Serial.println(F("  eyecolor <0-19>    - Set primary eye color"));
    Serial.println(F("  eyecolor2 <0-19>   - Set secondary eye color"));
    Serial.println(F("  eyemode <0-4>      - Set eye mode"));
    Serial.println(F("    0=Single, 1=Dual, 2=Alternating, 3=Rainbow, 4=Audio"));
    Serial.println(F("  eyebrightness <50-200> - Eye brightness %"));
    
    // NEW: Eye flicker commands
    Serial.println(F("  eyeflicker on/off  - Enable/disable eye flicker"));
    Serial.println(F("  eyeflickertime <min> <max> - Set flicker timing (ms)"));
    Serial.println(F("  eyestaticbright <0-255> - Set brightness when flicker off"));
    Serial.println(F("  eyeflicker settings - Show eye flicker configuration"));
    
    printSection("Mouth Commands");
    Serial.println(F("  mouth <0-14>       - Set mouth pattern"));
    Serial.println(F("  mouthcolor <0-19>  - Set primary mouth color"));
    Serial.println(F("  mouthcolor2 <0-19> - Set secondary mouth color"));
    Serial.println(F("  mouthsplit <0-4>   - Set mouth split mode"));
    Serial.println(F("  mouthbrightness <1-255> - Set mouth brightness"));
    Serial.println(F("  mouthenable on/off - Enable/disable mouth"));
    Serial.println(F("  talkspeed <1-10>   - Set talk animation speed"));
    Serial.println(F("  smilewidth <2-10>  - Set smile width"));
    Serial.println(F("  wavespeed <1-10>   - Set wave animation speed"));
    Serial.println(F("  pulsespeed <1-10>  - Set pulse animation speed"));
    
    printSection("Audio Commands");
    Serial.println(F("  audiomode <0-5>    - Set audio routing mode"));
    Serial.println(F("    0=Off, 1=Mouth Only, 2=Body Sides, 3=Body All"));
    Serial.println(F("    4=Everything, 5=Frequency Bands"));
    Serial.println(F("  audiosens <1-10>   - Set audio sensitivity"));
    Serial.println(F("  audiothreshold <50-500> - Set threshold manually"));
    Serial.println(F("  autogain on/off    - Enable/disable auto gain"));
    Serial.println(F("  resetpeak          - Reset audio peak level"));
    
    printSection("Random Blocks Configuration");
    Serial.println(F("  blockcolor <0-8> <0-19> - Set block color"));
    Serial.println(F("  blockrate <1-255>  - Set block blink speed"));
    Serial.println(F("  sidecolors <0-19> <0-19> <0-19> - Set side colors"));
    Serial.println(F("  sidemode <0-4>     - Set side color mode"));
    Serial.println(F("  siderate <1-255>   - Set side blink speed"));
    Serial.println(F("  showblocks         - Show block assignments"));
    
    printSection("Pattern Color Commands");
    Serial.println(F("  color <0-19>       - Solid color"));
    Serial.println(F("  solidmode <0-1>    - Solid mode (0=Static, 1=Blink)"));
    Serial.println(F("  flashcolor <0-19>  - Flash color"));
    Serial.println(F("  flashspeed <1-10>  - Flash speed"));
    Serial.println(F("  shortcolor <0-19>  - Short circuit color"));
    Serial.println(F("  knightcolor <0-19> - Knight Rider color"));
    Serial.println(F("  breathcolor <0-19> - Breathing color"));
    Serial.println(F("  matrixcolor <0-19> - Matrix rain color"));
    Serial.println(F("  strobecolor <0-19> - Strobe color"));
    Serial.println(F("  confetti <0-19> <0-19> - Confetti colors"));
    
    printSection("Demo Mode");
    Serial.println(F("  demo on/off        - Demo mode"));
    Serial.println(F("  demotime <5-300>   - Demo interval (seconds)"));
    
    printSection("Settings");
    Serial.println(F("  brightness <1-255> - Global brightness"));
    Serial.println(F("  bodybrightness <50-200> - Body brightness %"));
    Serial.println(F("  mouthouter <50-200> - Mouth outer LED boost %"));
    Serial.println(F("  mouthinner <50-200> - Mouth inner LED boost %"));
    Serial.println(F("  speed <1-255>      - Effect speed"));
    Serial.println(F("  fade <1-50>        - Fade speed"));
    Serial.println(F("  sidetime <min> <max> - Side LED timing"));
    Serial.println(F("  blocktime <min> <max> - Block timing"));
    
    printSection("System");
    Serial.println(F("  save               - Save to flash"));
    Serial.println(F("  load               - Load from flash"));
    Serial.println(F("  reset              - Factory reset"));
    Serial.println(F("  status             - Show settings"));
    Serial.println(F("  sysinfo            - System information"));
    Serial.println(F("  debug              - Debug information"));
    Serial.println(F("  validate           - Validate system state"));
    Serial.println(F("  reboot             - Restart device"));
    Serial.println(F("  help               - This help"));
    
    printSection("User Presets");
    Serial.println(F("  saveuser <1-10> [name] - Save current as preset"));
    Serial.println(F("  loaduser <1-10>    - Load user preset"));
    Serial.println(F("  deleteuser <1-10>  - Delete user preset"));
    Serial.println(F("  listpresets        - Show saved presets"));
    Serial.println(F("  presetinfo <1-10>  - Show preset details"));
    Serial.println(F("  validatepresets    - Check all presets"));
       
    printSection("Available Patterns");
    for (int i = 0; i < NUM_PATTERNS; i++) {
        Serial.print(F("  "));
        if (i < 10) Serial.print(F(" "));
        Serial.print(i);
        Serial.print(F(": "));
        Serial.println(patternNames[i]);
    }
    
    printSection("Available Colors");
    for (int i = 0; i < NUM_STANDARD_COLORS; i++) {
        if (i % 4 == 0) Serial.print(F("  "));
        Serial.print(i);
        if (i < 10) Serial.print(F(" "));
        Serial.print(F(":"));
        Serial.print(ColorNames[i]);
        if (i < NUM_STANDARD_COLORS - 1) {
            Serial.print(F(", "));
            if ((i + 1) % 4 == 0) Serial.println();
        }
    }
    Serial.println();
    
    // Command statistics
    printSection("Command Statistics");
    Serial.print(F("  Total Commands: "));
    Serial.println(totalCommands);
    Serial.print(F("  Failed Commands: "));
    Serial.println(failedCommands);
    Serial.print(F("  Success Rate: "));
    if (totalCommands > 0) {
        Serial.print(((totalCommands - failedCommands) * 100) / totalCommands);
        Serial.println(F("%"));
    } else {
        Serial.println(F("N/A"));
    }
    
    printSeparator();
}

void printCurrentSettings() {
    printSeparator();
    Serial.println(F("Current Settings"));
    printSeparator();
    
    // Pattern info
    Serial.print(F("Pattern: "));
    Serial.print(currentPattern);
    Serial.print(F(" - "));
    Serial.println(patternNames[currentPattern]);
    
    // Pattern manager status
    if (patternManager.isTransitioning()) {
        Serial.print(F("  Transition Progress: "));
        Serial.print(patternManager.getTransitionProgress());
        Serial.println(F("/255"));
    }
    
    // Demo mode
    Serial.print(F("Demo Mode: "));
    if (demoMode) {
        Serial.print(F("ON ("));
        Serial.print(demoTime);
        Serial.println(F("s interval)"));
    } else {
        Serial.println(F("OFF"));
    }
    
    // Audio
    printSection("Audio");
    Serial.print(F("  Mode: "));
    Serial.print(audioMode);
    Serial.print(F(" - "));
    Serial.println(AudioModeNames[audioMode]);
    Serial.print(F("  Sensitivity: "));
    Serial.print(audioSensitivity);
    Serial.print(F("/10, Threshold: "));
    Serial.print(audioThreshold);
    Serial.print(F(", Auto Gain: "));
    Serial.println(audioAutoGain ? "ON" : "OFF");
    Serial.print(F("  Current Level: "));
    Serial.print(audioProcessor.getLevel());
    Serial.print(F(", Peak: "));
    Serial.print(audioProcessor.getPeakLevel());
    Serial.print(F(", Average: "));
    Serial.print((int)audioProcessor.getAverageLevel());
    Serial.print(F(", BPM: "));
    Serial.println(audioProcessor.getBPM());
    Serial.print(F("  Gain Multiplier: "));
    Serial.print(audioProcessor.getGainMultiplier(), 2);
    Serial.print(F(", Beat Detected: "));
    Serial.println(audioProcessor.isBeatDetected() ? "YES" : "NO");
    
    // Eyes - ENHANCED with flicker info
    printSection("Eyes");
    Serial.print(F("  Mode: "));
    Serial.print(EyeModeNames[eyeMode]);
    Serial.print(F(", Colors: "));
    Serial.print(ColorNames[eyeColorIndex]);
    if (eyeMode != EYE_MODE_SINGLE) {
        Serial.print(F(" / "));
        Serial.print(ColorNames[eyeColorIndex2]);
    }
    Serial.println();
    Serial.print(F("  Brightness: "));
    Serial.print(eyeBrightness);
    Serial.println(F("%"));

    Serial.print(F("  Flicker: "));
    Serial.println(eyeFlickerEnabled ? "ON" : "OFF");
    if (eyeFlickerEnabled) {
        Serial.print(F("  Timing: "));
        Serial.print(eyeFlickerMinTime);
        Serial.print(F("-"));
        Serial.print(eyeFlickerMaxTime);
        Serial.println(F("ms"));
    } else {
        Serial.print(F("  Static Brightness: "));
        Serial.println(eyeStaticBrightness);
    }
    
    // Mouth
    printSection("Mouth");
    Serial.print(F("  Pattern: "));
    Serial.print(mouthPattern);
    Serial.print(F(" - "));
    Serial.println(MouthPatternNames[mouthPattern]);
    Serial.print(F("  Color: "));
    Serial.print(ColorNames[mouthColorIndex]);
    if (mouthSplitMode != MOUTH_SPLIT_OFF) {
        Serial.print(F(" / "));
        Serial.print(ColorNames[mouthColorIndex2]);
        Serial.print(F(" ("));
        Serial.print(MouthSplitModeNames[mouthSplitMode]);
        Serial.print(F(")"));
    }
    Serial.println();
    Serial.print(F("  Brightness: "));
    Serial.print(mouthBrightness);
    Serial.print(F(", Enabled: "));
    Serial.print(mouthEnabled ? "YES" : "NO");
    Serial.print(F(", Outer Boost: "));
    Serial.print(mouthOuterBoost);
    Serial.print(F("%, Inner Boost: "));
    Serial.print(mouthInnerBoost);
    Serial.println(F("%"));
    
    // Brightness
    printSection("Brightness");
    Serial.print(F("  Global: "));
    Serial.print(ledBrightness);
    Serial.print(F(", Eye: "));
    Serial.print(eyeBrightness);
    Serial.print(F("%, Body: "));
    Serial.print(bodyBrightness);
    Serial.println(F("%"));
    
    // Timing
    printSection("Timing");
    Serial.print(F("  Effect Speed: "));
    Serial.print(effectSpeed);
    Serial.print(F(", Fade: "));
    Serial.println(fadeSpeed);
    Serial.print(F("  Side Time: "));
    Serial.print(sideMinTime);
    Serial.print(F("-"));
    Serial.print(sideMaxTime);
    Serial.print(F("ms, Block Time: "));
    Serial.print(blockMinTime);
    Serial.print(F("-"));
    Serial.print(blockMaxTime);
    Serial.println(F("ms"));
    
    // Side LED configuration
    printSection("Side LEDs");
    Serial.print(F("  Mode: "));
    Serial.print(SideColorModeNames[sideColorMode]);
    Serial.print(F(", Colors: "));
    Serial.print(ColorNames[sideColor1]);
    Serial.print(F(", "));
    Serial.print(ColorNames[sideColor2]);
    Serial.print(F(", "));
    Serial.println(ColorNames[sideColor3]);
    Serial.print(F("  Rates: Side="));
    Serial.print(sideBlinkRate);
    Serial.print(F(", Block="));
    Serial.println(blockBlinkRate);
}

void printBlockColors() {
    printSection("Block Color Assignment");
    const char* boardNames[3] = {"Left  ", "Middle", "Right "};
    
    // Visual representation
    Serial.println(F("  [B1] [B2] [B3]   [B1] [B2] [B3]   [B1] [B2] [B3]"));
    Serial.print(F("  "));
    
    // Print block colors with bounds checking
    for (int board = 0; board < 3; board++) {
        for (int block = 0; block < 3; block++) {
            int blockIndex = (2 - board) * 3 + block;
            if (blockIndex < 9) { // Safety check
                Serial.print(F("["));
                if (blockColors[blockIndex] < 10) Serial.print(F(" "));
                Serial.print(blockColors[blockIndex]);
                Serial.print(F("] "));
            }
        }
        Serial.print(F("  "));
    }
    Serial.println();
    Serial.println(F("  Left      Middle     Right"));
    
    // Detailed list with validation
    Serial.println();
    for (int board = 0; board < 3; board++) {
        Serial.print(F("  "));
        Serial.print(boardNames[board]);
        Serial.print(F(": "));
        for (int block = 0; block < 3; block++) {
            int blockIndex = (2 - board) * 3 + block;
            if (blockIndex < 9) {
                Serial.print(F("B"));
                Serial.print(block + 1);
                Serial.print(F("="));
                
                uint8_t colorIdx = blockColors[blockIndex];
                if (colorIdx < NUM_STANDARD_COLORS || colorIdx == 10) {
                    Serial.print(ColorNames[colorIdx]);
                } else {
                    Serial.print(F("INVALID("));
                    Serial.print(colorIdx);
                    Serial.print(F(")"));
                }
                
                if (block < 2) Serial.print(F(", "));
            }
        }
        Serial.println();
    }
}

void printSystemInfo() {
    printSection("System Information");
    Serial.print(F("  Firmware: "));
    Serial.print(FIRMWARE_VERSION);
    Serial.print(F(" ("));
    Serial.print(FIRMWARE_DATE);
    Serial.println(F(")"));
    
    Serial.print(F("  Uptime: "));
    unsigned long uptime = millis() / 1000;
    Serial.print(uptime / 3600);
    Serial.print(F("h "));
    Serial.print((uptime % 3600) / 60);
    Serial.print(F("m "));
    Serial.print(uptime % 60);
    Serial.println(F("s"));
    
    Serial.print(F("  Free Heap: "));
    Serial.print(ESP.getFreeHeap());
    Serial.print(F(" bytes (Min: "));
    Serial.print(ESP.getMinFreeHeap());
    Serial.print(F(", Largest Block: "));
    Serial.print(ESP.getMaxAllocHeap());
    Serial.println(F(")"));
    
    // Thread-safe LED FPS access
    Serial.print(F("  LED FPS: "));
    if (acquireLEDMutex(50, "printSystemInfo")) {
        Serial.print(FastLED.getFPS());
        releaseLEDMutex("printSystemInfo");
    } else {
        Serial.print(F("N/A"));
    }
    Serial.println();
    
    Serial.print(F("  CPU Frequency: "));
    Serial.print(ESP.getCpuFreqMHz());
    Serial.println(F(" MHz"));
    
    Serial.print(F("  Flash Size: "));
    Serial.print(ESP.getFlashChipSize() / 1024 / 1024);
    Serial.println(F(" MB"));
    
    Serial.print(F("  Chip Model: "));
    Serial.println(ESP.getChipModel());
    
    Serial.print(F("  Chip Revision: "));
    Serial.println(ESP.getChipRevision());
    
    Serial.print(F("  SDK Version: "));
    Serial.println(ESP.getSdkVersion());
    
    // Task information
    Serial.print(F("  FreeRTOS Tasks: "));
    Serial.println(uxTaskGetNumberOfTasks());
    
    // System status
    Serial.print(F("  System Ready: "));
    Serial.println(systemReady ? "YES" : "NO");
    
    Serial.print(F("  Boot Time: "));
    Serial.print(bootTime / 1000);
    Serial.println(F("s"));
}

void printDebugInfo() {
    printSection("Debug Information");
    
    // Pattern Manager Status
    Serial.println(F("Pattern Manager:"));
    patternManager.printStatus();
    
    // Preset Manager Statistics
    Serial.println(F("Preset Manager:"));
    presetManager.printStatistics();
    
    // LED Performance
    printLEDPerformanceStats();
    
    // Audio Processor Status
    Serial.println(F("Audio Processor:"));
    Serial.print(F("  Current Level: "));
    Serial.println(audioProcessor.getLevel());
    Serial.print(F("  Peak Level: "));
    Serial.println(audioProcessor.getPeakLevel());
    Serial.print(F("  Average Level: "));
    Serial.println(audioProcessor.getAverageLevel());
    Serial.print(F("  Bass Level: "));
    Serial.println(audioProcessor.getBassLevel());
    Serial.print(F("  Mid Level: "));
    Serial.println(audioProcessor.getMidLevel());
    Serial.print(F("  Treble Level: "));
    Serial.println(audioProcessor.getTrebleLevel());
    Serial.print(F("  BPM: "));
    Serial.println(audioProcessor.getBPM());
    Serial.print(F("  Gain Multiplier: "));
    Serial.println(audioProcessor.getGainMultiplier(), 3);
    
    // Memory analysis
    Serial.println(F("Memory Analysis:"));
    Serial.print(F("  Stack High Water Mark: "));
    Serial.println(uxTaskGetStackHighWaterMark(NULL));
    
    // Event Logger
    Serial.print(F("Event Logger Entries: "));
    Serial.println(eventLogger.getCount());
}

// Safe integer parsing with comprehensive error handling
int safeParseInt(const String& str, int defaultValue = -1, int minValue = INT_MIN, int maxValue = INT_MAX) {
    // Input validation
    if (str.length() == 0) {
        return defaultValue;
    }
    
    if (str.length() > 10) { // Reasonable limit for integer strings
        Serial.print(F("! Number too long: "));
        Serial.println(str);
        return defaultValue;
    }
    
    // Copy to buffer for safe processing
    if (str.length() >= sizeof(tempBuffer)) {
        return defaultValue;
    }
    
    str.toCharArray(tempBuffer, sizeof(tempBuffer));
    
    // Check format: optional minus, then digits only
    bool negative = false;
    int startPos = 0;
    
    if (tempBuffer[0] == '-') {
        negative = true;
        startPos = 1;
    } else if (tempBuffer[0] == '+') {
        startPos = 1;
    }
    
    // Must have at least one digit after sign
    if (tempBuffer[startPos] == '\0') {
        return defaultValue;
    }
    
    // Validate all remaining characters are digits
    for (int i = startPos; tempBuffer[i] != '\0'; i++) {
        if (!isdigit(tempBuffer[i])) {
            Serial.print(F("! Invalid character in number: '"));
            Serial.print(tempBuffer[i]);
            Serial.println(F("'"));
            return defaultValue;
        }
    }
    
    // Parse with overflow detection
    long longValue = str.toInt();
    
    // Check for overflow/underflow
    if (longValue < (long)minValue || longValue > (long)maxValue) {
        Serial.print(F("! Number out of range: "));
        Serial.print(longValue);
        Serial.print(F(" (allowed: "));
        Serial.print(minValue);
        Serial.print(F(" to "));
        Serial.print(maxValue);
        Serial.println(F(")"));
        return defaultValue;
    }
    
    return (int)longValue;
}

// Safe string parameter extraction
bool extractStringParams(const String& input, String* params, int maxParams, int& paramCount) {
    paramCount = 0;
    if (maxParams <= 0) return false;
    
    int startPos = 0;
    int spacePos = 0;
    
    while (paramCount < maxParams && startPos < (int)input.length()) {
        spacePos = input.indexOf(' ', startPos);
        
        if (spacePos == -1) {
            // Last parameter
            params[paramCount] = input.substring(startPos);
            params[paramCount].trim();
            if (params[paramCount].length() > 0) {
                paramCount++;
            }
            break;
        } else {
            // Extract parameter
            params[paramCount] = input.substring(startPos, spacePos);
            params[paramCount].trim();
            if (params[paramCount].length() > 0) {
                paramCount++;
            }
            startPos = spacePos + 1;
        }
    }
    
    return paramCount > 0;
}

void processSerialCommand() {
    totalCommands++;
    unsigned long commandProcessingStart = millis();
    
    // NEU: Eine do-while(false)-Schleife umschließt die gesamte Logik.
    // Dies ermöglicht es uns, mit 'break' sicher aus dem Block zu springen.
    do {
        // Input validation and sanitization
        inputString.trim();
        
        if (inputString.length() == 0) {
            Serial.println(F("! Empty command"));
            failedCommands++;
            break; // ERSETZT: goto cleanup;
        }
        
        // Check for command repetition (potential spam/loop)
        if (inputString == lastCommand && millis() - lastCommandTime < 1000) {
            repeatCommandCount++;
            if (repeatCommandCount > 5) {
                Serial.println(F("! Command repeated too many times - ignoring"));
                failedCommands++;
                break; // ERSETZT: goto cleanup;
            }
        } else {
            lastCommand = inputString;
            repeatCommandCount = 0;
        }
        lastCommandTime = millis();
        
        // Store original for error reporting
        String originalCommand = inputString;
        inputString.toLowerCase();
        
        // Basic command processing with comprehensive error handling
        bool commandFound = false;
        
        // Single-word commands
        if (inputString == "help" || inputString == "h") {
            printHelp();
            commandFound = true;
        }
        else if (inputString == "status") {
            printCurrentSettings();
            commandFound = true;
        }
        else if (inputString == "sysinfo") {
            printSystemInfo();
            commandFound = true;
        }
        else if (inputString == "debug") {
            printDebugInfo();
            commandFound = true;
        }
        else if (inputString == "showblocks") {
            printBlockColors();
            commandFound = true;
        }
        else if (inputString == "validate") {
            Serial.println(F("> Running system validation..."));
            validateHelperState();
            uint8_t corruptedPresets = presetManager.validateAllPresets();
            if (corruptedPresets > 0) {
                Serial.print(F("> Found "));
                Serial.print(corruptedPresets);
                Serial.println(F(" corrupted presets"));
            } else {
                Serial.println(F("> System validation complete - no issues found"));
            }
            commandFound = true;
        }
        else if (inputString == "save") {
            saveSettings();
            Serial.println(F("> Settings saved to flash"));
            commandFound = true;
        }
        else if (inputString == "load") {
            loadSettings();
            Serial.println(F("> Settings loaded from flash"));
            commandFound = true;
        }
        else if (inputString == "reset") {
            Serial.println(F("> Resetting to factory defaults..."));
            resetToDefaults();
            saveSettings();
            Serial.println(F("> Factory reset complete"));
            commandFound = true;
        }
        else if (inputString == "reboot") {
            Serial.println(F("> Rebooting in 3 seconds..."));
            // Save current state before reboot
            saveSettings();
            delay(3000);
            ESP.restart();
        }
        
        // Pattern commands
        else if (inputString.startsWith("s ")) {
            int pattern = safeParseInt(inputString.substring(2), -1, 0, NUM_PATTERNS - 1);
            if (pattern >= 0) {
                PatternChangeResult result = patternManager.changePattern(pattern);
                if (result == PATTERN_CHANGE_SUCCESS) {
                    Serial.print(F("> Pattern changed to: "));
                    Serial.print(pattern);
                    Serial.print(F(" - "));
                    Serial.println(patternNames[pattern]);
                } else {
                    Serial.print(F("! Pattern change failed (error "));
                    Serial.print((int)result);
                    Serial.println(F(")"));
                    failedCommands++;
                }
            } else {
                Serial.print(F("! Invalid pattern! Use 0-"));
                Serial.println(NUM_PATTERNS - 1);
                failedCommands++;
            }
            commandFound = true;
        }
        else if (inputString == "next") {
            PatternChangeResult result = patternManager.nextPattern();
            if (result == PATTERN_CHANGE_SUCCESS) {
                Serial.print(F("> Next pattern: "));
                Serial.print(currentPattern);
                Serial.print(F(" - "));
                Serial.println(patternNames[currentPattern]);
            } else {
                Serial.println(F("! Failed to change to next pattern"));
                failedCommands++;
            }
            commandFound = true;
        }
        else if (inputString == "prev") {
            PatternChangeResult result = patternManager.previousPattern();
            if (result == PATTERN_CHANGE_SUCCESS) {
                Serial.print(F("> Previous pattern: "));
                Serial.print(currentPattern);
                Serial.print(F(" - "));
                Serial.println(patternNames[currentPattern]);
            } else {
                Serial.println(F("! Failed to change to previous pattern"));
                failedCommands++;
            }
            commandFound = true;
        }
        
        // Eye commands
        else if (inputString.startsWith("eyecolor ")) {
            int color = safeParseInt(inputString.substring(9), -1, 0, NUM_STANDARD_COLORS - 1);
            if (color >= 0) {
                eyeColorIndex = color;
                Serial.print(F("> Eye color set to: "));
                Serial.println(ColorNames[color]);
            } else {
                Serial.println(F("! Invalid color index"));
                failedCommands++;
            }
            commandFound = true;
        }
        else if (inputString.startsWith("eyecolor2 ")) {
            int color = safeParseInt(inputString.substring(10), -1, 0, NUM_STANDARD_COLORS - 1);
            if (color >= 0) {
                eyeColorIndex2 = color;
                Serial.print(F("> Eye color 2 set to: "));
                Serial.println(ColorNames[color]);
            } else {
                Serial.println(F("! Invalid color index"));
                failedCommands++;
            }
            commandFound = true;
        }
        else if (inputString.startsWith("eyemode ")) {
            int mode = safeParseInt(inputString.substring(8), -1, 0, 4);
            if (mode >= 0) {
                eyeMode = mode;
                Serial.print(F("> Eye mode set to: "));
                Serial.println(EyeModeNames[mode]);
            } else {
                Serial.println(F("! Invalid eye mode (0-4)"));
                failedCommands++;
            }
            commandFound = true;
        }
        // NEW: Eye flicker commands
        else if (inputString == "eyeflicker on") {
            eyeFlickerEnabled = true;
            Serial.println(F("> Eye flicker enabled"));
            commandFound = true;
        }
        else if (inputString == "eyeflicker off") {
            eyeFlickerEnabled = false;
            Serial.println(F("> Eye flicker disabled - eyes now static"));
            commandFound = true;
        }
        else if (inputString.startsWith("eyeflickertime ")) {
            String params[2];
            int paramCount;
            
            if (extractStringParams(inputString.substring(15), params, 2, paramCount) && paramCount == 2) {
                int minTime = safeParseInt(params[0], -1, 50, 5000);
                int maxTime = safeParseInt(params[1], -1, 50, 5000);
                
                if (minTime > 0 && maxTime > 0) {
                    if (maxTime <= minTime) {
                        Serial.println(F("! Max time must be greater than min time"));
                        failedCommands++;
                    } else {
                        eyeFlickerMinTime = minTime;
                        eyeFlickerMaxTime = maxTime;
                        Serial.print(F("> Eye flicker timing set to "));
                        Serial.print(minTime);
                        Serial.print(F("-"));
                        Serial.print(maxTime);
                        Serial.println(F("ms"));
                    }
                } else {
                    Serial.println(F("! Invalid timing values (50-5000ms)"));
                    failedCommands++;
                }
            } else {
                Serial.println(F("! Usage: eyeflickertime <min> <max>"));
                failedCommands++;
            }
            commandFound = true;
        }
        else if (inputString.startsWith("eyestaticbright ")) {
            int brightness = safeParseInt(inputString.substring(16), -1, 0, 255);
            if (brightness >= 0) {
                eyeStaticBrightness = brightness;
                Serial.print(F("> Eye static brightness set to: "));
                Serial.println(brightness);
            } else {
                Serial.println(F("! Invalid brightness (0-255)"));
                failedCommands++;
            }
            commandFound = true;
        }
        else if (inputString == "eyeflicker settings") {
            printEyeFlickerSettings();
            commandFound = true;
        }
        
        // Audio commands
        else if (inputString.startsWith("audiomode ")) {
            int mode = safeParseInt(inputString.substring(10), -1, 0, 5);
            if (mode >= 0) {
                audioMode = mode;
                Serial.print(F("> Audio mode set to: "));
                Serial.println(AudioModeNames[mode]);
            } else {
                Serial.println(F("! Invalid audio mode (0-5)"));
                failedCommands++;
            }
            commandFound = true;
        }
        else if (inputString == "resetpeak") {
            audioProcessor.resetPeak();
            Serial.println(F("> Audio peak reset"));
            commandFound = true;
        }
        
        // Brightness commands  
        else if (inputString.startsWith("brightness ")) {
            int bright = safeParseInt(inputString.substring(11), -1, 1, 255);
            if (bright > 0) {
                ledBrightness = bright;
                FastLED.setBrightness(ledBrightness);
                Serial.print(F("> Brightness set to: "));
                Serial.println(bright);
            } else {
                Serial.println(F("! Brightness must be 1-255"));
                failedCommands++;
            }
            commandFound = true;
        }
        
        // Demo mode
        else if (inputString == "demo on") {
            demoMode = true;
            Serial.println(F("> Demo mode enabled"));
            commandFound = true;
        }
        else if (inputString == "demo off") {
            demoMode = false;
            Serial.println(F("> Demo mode disabled"));
            commandFound = true;
        }
        
        // Preset commands
        else if (inputString.startsWith("saveuser ")) {
            String params[2];
            int paramCount;
            
            if (extractStringParams(inputString.substring(9), params, 2, paramCount)) {
                int slot = safeParseInt(params[0], -1, 1, MAX_PRESETS);
                if (slot > 0) {
                    String presetName = "Preset " + String(slot);
                    if (paramCount > 1 && params[1].length() > 0) {
                        presetName = params[1];
                        if (presetName.length() >= PRESET_NAME_LENGTH) {
                            presetName = presetName.substring(0, PRESET_NAME_LENGTH - 1);
                            Serial.println(F("! Preset name truncated"));
                        }
                    }
                    
                    PresetResult result = presetManager.savePreset(slot - 1, presetName.c_str());
                    if (result == PRESET_SUCCESS) {
                        Serial.print(F("> User preset "));
                        Serial.print(slot);
                        Serial.print(F(" saved as '"));
                        Serial.print(presetName);
                        Serial.println(F("'"));
                    } else {
                        Serial.print(F("! Failed to save preset (error "));
                        Serial.print((int)result);
                        Serial.println(F(")"));
                        failedCommands++;
                    }
                } else {
                    Serial.print(F("! Invalid preset slot (1-"));
                    Serial.print(MAX_PRESETS);
                    Serial.println(F(")"));
                    failedCommands++;
                }
            } else {
                Serial.println(F("! Usage: saveuser <1-10> [name]"));
                failedCommands++;
            }
            commandFound = true;
        }
        else if (inputString.startsWith("loaduser ")) {
            int slot = safeParseInt(inputString.substring(9), -1, 1, MAX_PRESETS);
            if (slot > 0) {
                PresetResult result = presetManager.loadPreset(slot - 1);
                if (result == PRESET_SUCCESS) {
                    Serial.print(F("> User preset "));
                    Serial.print(slot);
                    Serial.println(F(" loaded"));
                } else {
                    Serial.print(F("! Failed to load preset (error "));
                    Serial.print((int)result);
                    Serial.println(F(")"));
                    failedCommands++;
                }
            } else {
                Serial.print(F("! Invalid preset slot (1-"));
                Serial.print(MAX_PRESETS);
                Serial.println(F(")"));
                failedCommands++;
            }
            commandFound = true;
        }
        else if (inputString.startsWith("deleteuser ")) {
            int slot = safeParseInt(inputString.substring(11), -1, 1, MAX_PRESETS);
            if (slot > 0) {
                PresetResult result = presetManager.deletePreset(slot - 1);
                if (result == PRESET_SUCCESS) {
                    Serial.print(F("> User preset "));
                    Serial.print(slot);
                    Serial.println(F(" deleted"));
                } else {
                    Serial.print(F("! Failed to delete preset (error "));
                    Serial.print((int)result);
                    Serial.println(F(")"));
                    failedCommands++;
                }
            } else {
                Serial.print(F("! Invalid preset slot (1-"));
                Serial.print(MAX_PRESETS);
                Serial.println(F(")"));
                failedCommands++;
            }
            commandFound = true;
        }
        else if (inputString == "listpresets") {
            Serial.println(F("> Saved Presets:"));
            bool foundAny = false;
            for (int i = 0; i < MAX_PRESETS; i++) {
                if (presetManager.isSlotUsed(i)) {
                    Serial.print(F("  Slot "));
                    Serial.print(i + 1);
                    Serial.print(F(": "));
                    Serial.print(presetManager.getPresetName(i));
                    Serial.print(F(" ("));
                    Serial.print((millis() - presetManager.getPresetTimestamp(i)) / 1000);
                    Serial.println(F("s ago)"));
                    foundAny = true;
                }
            }
            if (!foundAny) {
                Serial.println(F("  No presets saved"));
            }
            commandFound = true;
        }
        else if (inputString.startsWith("presetinfo ")) {
            int slot = safeParseInt(inputString.substring(11), -1, 1, MAX_PRESETS);
            if (slot > 0) {
                presetManager.printSlotInfo(slot - 1);
            } else {
                Serial.print(F("! Invalid preset slot (1-"));
                Serial.print(MAX_PRESETS);
                Serial.println(F(")"));
                failedCommands++;
            }
            commandFound = true;
        }
        else if (inputString == "testleds") {
            Serial.println(F("> Testing LED chaining configuration..."));
            
            // Clear all LEDs
            fill_solid(bodyLEDsChained, TOTAL_BODY_LEDS, CRGB::Black);
            fill_solid(eyesMouthLEDs, NUM_EYES + NUM_MOUTH_LEDS, CRGB::Black);
            FastLED.show();
            delay(500);
            
            // Test body panels sequentially
            Serial.println(F("  Testing Right panel (red)"));
            fill_solid(DJLEDs_Right, NUM_LEDS_PER_PANEL, CRGB::Red);
            FastLED.show();
            delay(1500);
            fill_solid(DJLEDs_Right, NUM_LEDS_PER_PANEL, CRGB::Black);
            
            Serial.println(F("  Testing Middle panel (green)"));
            fill_solid(DJLEDs_Middle, NUM_LEDS_PER_PANEL, CRGB::Green);
            FastLED.show();
            delay(1500);
            fill_solid(DJLEDs_Middle, NUM_LEDS_PER_PANEL, CRGB::Black);
            
            Serial.println(F("  Testing Left panel (blue)"));
            fill_solid(DJLEDs_Left, NUM_LEDS_PER_PANEL, CRGB::Blue);
            FastLED.show();
            delay(1500);
            fill_solid(DJLEDs_Left, NUM_LEDS_PER_PANEL, CRGB::Black);
            
            // Test mouth and eyes
            Serial.println(F("  Testing Mouth (yellow)"));
            fill_solid(DJLEDs_Mouth, NUM_MOUTH_LEDS, CRGB::Yellow);
            FastLED.show();
            delay(1500);
            fill_solid(DJLEDs_Mouth, NUM_MOUTH_LEDS, CRGB::Black);
            
            Serial.println(F("  Testing Eyes (white)"));
            fill_solid(DJLEDs_Eyes, NUM_EYES, CRGB::White);
            FastLED.show();
            delay(1500);
            fill_solid(DJLEDs_Eyes, NUM_EYES, CRGB::Black);
            
            FastLED.show();
            Serial.println(F("> LED chaining test complete"));
            commandFound = true;
        }
        else if (inputString == "validatehardware") {
            validateHardwareConfiguration();
            commandFound = true;
        }
        
        // Handle unrecognized commands
        if (!commandFound) {
            Serial.print(F("! Unknown command: '"));
            Serial.print(originalCommand);
            Serial.println(F("'"));
            Serial.println(F("  Type 'help' for available commands"));
            failedCommands++;
        }
    } while(false);
    
    // Command cleanup
    inputString = "";
    stringComplete = false;
    commandStartTime = 0;
    commandInProgress = false;
    
    // Performance logging
    unsigned long processingTime = millis() - commandProcessingStart;
    if (processingTime > 100) { // Log slow commands
        Serial.print(F("! Command took "));
        Serial.print(processingTime);
        Serial.println(F("ms to process"));
    }
}

void resetSerialCommandStats() {
    totalCommands = 0;
    failedCommands = 0;
    lastCommand = "";
    lastCommandTime = 0;
    repeatCommandCount = 0;
    Serial.println(F("Serial command statistics reset"));
}

void printSerialCommandStats() {
    Serial.println(F("=== Serial Command Statistics ==="));
    Serial.print(F("Total Commands: "));
    Serial.println(totalCommands);
    Serial.print(F("Failed Commands: "));
    Serial.println(failedCommands);
    Serial.print(F("Success Rate: "));
    if (totalCommands > 0) {
        Serial.print(((totalCommands - failedCommands) * 100) / totalCommands);
        Serial.println(F("%"));
    } else {
        Serial.println(F("100%"));
    }
    Serial.print(F("Last Command: "));
    Serial.println(lastCommand.length() > 0 ? lastCommand : "None");
    Serial.print(F("Repeat Count: "));
    Serial.println(repeatCommandCount);
    Serial.println(F("================================"));
}