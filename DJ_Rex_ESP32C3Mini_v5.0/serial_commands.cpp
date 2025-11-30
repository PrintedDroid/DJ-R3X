#include "serial_commands.h"
#include "settings.h"
#include "helpers.h"
#include "eyes.h"  // Added for printEyeFlickerSettings()

// v5.0: New module includes
#include "event_logger.h"
#include "preset_manager.h"
#include "system_monitor.h"

// Serial input buffer
String inputString = "";
boolean stringComplete = false;

//Function to parse the playlist string
void parsePlaylistCommand(String command) {
    // Remove "playlist " part
    String data = command.substring(9);
    data.trim();

    // Clear current playlist
    playlistSize = 0;
    int entryIndex = 0;

    int currentPos = 0;
    while (currentPos < data.length() && entryIndex < 10) {
        int semicolonPos = data.indexOf(';', currentPos);
        String entryString;

        if (semicolonPos == -1) {
            entryString = data.substring(currentPos);
            currentPos = data.length();
        } else {
            entryString = data.substring(currentPos, semicolonPos);
            currentPos = semicolonPos + 1;
        }

        int commaPos = entryString.indexOf(',');
        if (commaPos > 0) {
            uint8_t pattern = entryString.substring(0, commaPos).toInt();
            uint16_t duration = entryString.substring(commaPos + 1).toInt();

            if (pattern < NUM_PATTERNS && duration > 0) {
                playlist[entryIndex].pattern = pattern;
                playlist[entryIndex].duration = duration;
                entryIndex++;
            }
        }
    }
    playlistSize = entryIndex;
    Serial.print(F("Playlist created with "));
    Serial.print(playlistSize);
    Serial.println(F(" entries."));
}

bool checkSerialCommand() {
    while (Serial.available()) {
        char inChar = (char)Serial.read();
        
        // Handle newline characters
        if (inChar == '\n' || inChar == '\r') {
            if (inputString.length() > 0) {  // Only process if we have content
                stringComplete = true;
                return true;
            }
        } else if (inChar >= 32 && inChar <= 126) {  // Only printable ASCII characters
            inputString += inChar;
        }
        
        // Prevent buffer overflow
        if (inputString.length() > 100) {
            inputString = "";
            Serial.println(F("Command too long - cleared"));
        }
    }
    return false;
}

void printHelp() {
    Serial.println(F("\n=== DJ Rex v5.0 - Command Reference ==="));
    Serial.println(F("Body Pattern Commands:"));
    Serial.println(F("  S <0-16>           - Set pattern"));
    Serial.println(F("  next/prev          - Navigate patterns"));
    Serial.println(F(""));
    Serial.println(F("Playlist Commands:"));
    Serial.println(F("  playlist on/off    - Enable/disable playlist mode"));
    Serial.println(F("  playlist show      - Show current playlist"));
    Serial.println(F("  playlist <p,d;p,d> - Set playlist (e.g., playlist 5,10;12,20)"));
    Serial.println(F(""));
    Serial.println(F("Eye Commands:"));
    Serial.println(F("  eyecolor <0-19>    - Set eye color"));
    Serial.println(F("  eyecolor2 <0-19>   - Set secondary eye color"));
    Serial.println(F("  eyemode <0-2>      - Set eye mode (0=Single, 1=Dual, 2=Alternating)"));
    Serial.println(F("  eyebrightness <50-200> - Set eye brightness %"));
    Serial.println(F("  eyeflicker on/off  - Enable/disable eye flicker"));
    Serial.println(F("  eyeflicker settings - Show eye flicker configuration"));
    Serial.println(F("  eyeflickertime <min> <max> - Set flicker timing (ms)"));
    Serial.println(F("  eyestaticbright <0-255> - Set brightness when flicker off"));
    Serial.println(F(""));
    Serial.println(F("Mouth Commands:"));
    Serial.println(F("  mouth <0-11>        - Set mouth pattern"));
    Serial.println(F("  mouthcolor <0-19>  - Set mouth color"));
    Serial.println(F("  mouthcolor2 <0-19> - Set secondary mouth color"));
    Serial.println(F("  mouthsplit <0-4>   - Set mouth split mode"));
    Serial.println(F("  wavespeed <1-10>   - Set wave animation speed"));
    Serial.println(F("  pulsespeed <1-10>  - Set pulse animation speed"));
    Serial.println(F("  mouthbrightness <1-255> - Set mouth brightness"));
    Serial.println(F("  mouthenable on/off - Enable/disable mouth"));
    Serial.println(F("  talkspeed <1-10>   - Set talk animation speed"));
    Serial.println(F("  smilewidth <2-10>  - Set smile width"));
    Serial.println(F(""));
    Serial.println(F("Audio Commands:"));
    Serial.println(F("  audiomode <0-4>    - Set audio routing mode"));
    Serial.println(F("    0=Off, 1=Mouth Only, 2=Body Sides, 3=Body All, 4=Everything"));
    Serial.println(F("  audiosens <1-10>   - Set audio sensitivity"));
    Serial.println(F("  audiothreshold <50-500> - Set threshold manually"));
    Serial.println(F("  autogain on/off    - Enable/disable auto gain"));
    Serial.println(F(""));
    Serial.println(F("Random Blocks Configuration:"));
    Serial.println(F("  blockcolor <0-8> <0-19> - Set block color"));
    Serial.println(F("  blockrate <1-255>  - Set block blink speed"));
    Serial.println(F("  sidecolors <0-19> <0-19> <0-19> - Set side colors"));
    Serial.println(F("  sidemode <0-4>     - Set side color mode"));
    Serial.println(F("  siderate <1-255>   - Set side blink speed"));
    Serial.println(F("  showblocks         - Show block assignments"));
    Serial.println(F(""));
    Serial.println(F("Pattern Color Commands:"));
    Serial.println(F("  color <0-19>       - Solid color"));
    Serial.println(F("  solidmode <0-1>    - Solid mode"));
    Serial.println(F("  flashcolor <0-19>  - Flash color"));
    Serial.println(F("  flashspeed <1-10>  - Flash speed"));
    Serial.println(F("  shortcolor <0-19>  - Short circuit color"));
    Serial.println(F("  knightcolor <0-19> - Knight Rider color"));
    Serial.println(F("  breathcolor <0-19> - Breathing color"));
    Serial.println(F("  matrixcolor <0-19> - Matrix rain color"));
    Serial.println(F("  strobecolor <0-19> - Strobe color"));
    Serial.println(F("  confetti <0-19> <0-19> - Confetti colors"));
    Serial.println(F(""));
    Serial.println(F("Demo Mode:"));
    Serial.println(F("  demo on/off        - Demo mode"));
    Serial.println(F("  demotime <5-300>   - Demo interval"));
    Serial.println(F(""));
    Serial.println(F("Settings:"));
    Serial.println(F("  brightness <1-255> - Global brightness"));
    Serial.println(F("  bodybrightness <50-200> - Body brightness %"));
    Serial.println(F("  mouthouter <50-200> - Mouth outer LED boost %"));
    Serial.println(F("  mouthinner <50-200> - Mouth inner LED boost %"));
    Serial.println(F("  speed <1-255>      - Effect speed"));
    Serial.println(F("  fade <1-50>        - Fade speed"));
    Serial.println(F("  sidetime <min> <max> - Side LED timing"));
    Serial.println(F("  blocktime <min> <max> - Block timing"));
    Serial.println(F(""));
    Serial.println(F("System:"));
    Serial.println(F("  save               - Save to flash"));
    Serial.println(F("  load               - Load from flash"));
    Serial.println(F("  reset              - Factory reset"));
    Serial.println(F("  status             - Show settings"));
    Serial.println(F("  help               - This help"));
    Serial.println(F(""));
    Serial.println(F("User Presets (v3.1 legacy):"));
    Serial.println(F("  saveuser <1-3>     - Save current setup as user preset"));
    Serial.println(F("  loaduser <1-3>     - Load user preset"));
    Serial.println(F("  deleteuser <1-3>   - Delete user preset"));
    Serial.println(F("  listpresets        - Show saved presets"));
    Serial.println(F(""));
    Serial.println(F("v5.0 Extended Presets (10 slots):"));
    Serial.println(F("  preset save <1-10> [name] - Save preset with optional name"));
    Serial.println(F("  preset load <1-10> - Load preset"));
    Serial.println(F("  preset delete <1-10> - Delete preset"));
    Serial.println(F("  preset list        - Show all presets"));
    Serial.println(F(""));
    Serial.println(F("v5.0 System Monitoring:"));
    Serial.println(F("  sysinfo            - Show system status (memory, health)"));
    Serial.println(F("  eventlog           - Show event log"));
    Serial.println(F("  eventlog clear     - Clear event log"));
    Serial.println(F(""));
    Serial.println(F("Patterns:"));
    for (int i = 0; i < NUM_PATTERNS; i++) {
        Serial.print(F("  "));
        Serial.print(i);
        Serial.print(F(": "));
        Serial.println(patternNames[i]);
    }
    Serial.println(F(""));
    Serial.println(F("Mouth Patterns:"));
    for (int i = 0; i < NUM_MOUTH_PATTERNS; i++) {
        Serial.print(F("  "));
        Serial.print(i);
        Serial.print(F(": "));
        Serial.println(MouthPatternNames[i]);
    }
    Serial.println(F(""));
    Serial.println(F("Audio Modes:"));
    for (int i = 0; i < 5; i++) {
        Serial.print(F("  "));
        Serial.print(i);
        Serial.print(F(": "));
        Serial.println(AudioModeNames[i]);
    }
    Serial.println(F(""));
    Serial.println(F("Colors (Extended Palette):"));
    for (int i = 0; i < NUM_STANDARD_COLORS; i++) {
        Serial.print(F("  "));
        if (i < 10) Serial.print(F(" "));  // Alignment for single digits
        Serial.print(i);
        Serial.print(F(": "));
        Serial.println(ColorNames[i]);
    }
    Serial.println(F(""));
    Serial.println(F("Block Layout:"));
    Serial.println(F("  Left:   0=B1, 1=B2, 2=B3"));
    Serial.println(F("  Middle: 3=B1, 4=B2, 5=B3"));
    Serial.println(F("  Right:  6=B1, 7=B2, 8=B3"));
    Serial.println(F("=====================================\n"));
}

void printCurrentSettings() {
    Serial.println(F("\n=== Current Settings ==="));
    Serial.print(F("Pattern: "));
    Serial.print(currentPattern);
    Serial.print(F(" ("));
    Serial.print(patternNames[currentPattern]);
    Serial.println(F(")"));
    
    Serial.print(F("Demo Mode: "));
    if (demoMode) {
        Serial.print(F("ON ("));
        Serial.print(demoTime);
        Serial.println(F("s)"));
    } else {
        Serial.println(F("OFF"));
    }
    
    Serial.println(F("\n--- Audio Settings ---"));
    Serial.print(F("Mode: "));
    Serial.print(audioMode);
    Serial.print(F(" ("));
    Serial.print(AudioModeNames[audioMode]);
    Serial.println(F(")"));
    Serial.print(F("Sensitivity: "));
    Serial.println(audioSensitivity);
    Serial.print(F("Threshold: "));
    Serial.println(audioThreshold);
    Serial.print(F("Auto Gain: "));
    Serial.println(audioAutoGain ? "ON" : "OFF");
    
    Serial.println(F("\n--- Eye Settings ---"));
    Serial.print(F("Mode: "));
    Serial.print(eyeMode);
    Serial.print(F(" ("));
    Serial.print(EyeModeNames[eyeMode]);
    Serial.println(F(")"));
    Serial.print(F("Color 1: "));
    Serial.print(eyeColorIndex);
    Serial.print(F(" ("));
    Serial.print(ColorNames[eyeColorIndex]);
    Serial.println(F(")"));
    Serial.print(F("Color 2: "));
    Serial.print(eyeColorIndex2);
    Serial.print(F(" ("));
    Serial.print(ColorNames[eyeColorIndex2]);
    Serial.println(F(")"));
    Serial.print(F("Brightness: "));
    Serial.print(eyeBrightness);
    Serial.println(F("%"));
    Serial.print(F("Flicker: "));
    Serial.println(eyeFlickerEnabled ? "ON" : "OFF");
    if (eyeFlickerEnabled) {
        Serial.print(F("  Timing: "));
        Serial.print(eyeFlickerMinTime);
        Serial.print(F("-"));
        Serial.print(eyeFlickerMaxTime);
        Serial.println(F("ms"));
    } else {
        Serial.print(F("  Static Brightness: "));
        Serial.print(eyeStaticBrightness);
        Serial.println(F("/255"));
    }
    
    Serial.println(F("\n--- Mouth Settings ---"));
    Serial.print(F("Enabled: "));
    Serial.println(mouthEnabled ? "Yes" : "No");
    Serial.print(F("Pattern: "));
    Serial.print(mouthPattern);
    Serial.print(F(" ("));
    Serial.print(MouthPatternNames[mouthPattern]);
    Serial.println(F(")"));
    Serial.print(F("Split Mode: "));
    Serial.print(mouthSplitMode);
    Serial.print(F(" ("));
    Serial.print(MouthSplitNames[mouthSplitMode]);
    Serial.println(F(")"));
    Serial.print(F("Color 1: "));
    Serial.print(mouthColorIndex);
    Serial.print(F(" ("));
    Serial.print(ColorNames[mouthColorIndex]);
    Serial.println(F(")"));
    Serial.print(F("Color 2: "));
    Serial.print(mouthColorIndex2);
    Serial.print(F(" ("));
    Serial.print(ColorNames[mouthColorIndex2]);
    Serial.println(F(")"));
    Serial.print(F("Brightness: "));
    Serial.println(mouthBrightness);
    
    if (currentPattern == 1) {
        Serial.println(F("\n--- Random Blocks Config ---"));
        Serial.print(F("Side Colors: "));
        Serial.print(ColorNames[sideColor1]);
        Serial.print(F(", "));
        Serial.print(ColorNames[sideColor2]);
        Serial.print(F(", "));
        Serial.println(ColorNames[sideColor3]);
        Serial.print(F("Side Mode: "));
        Serial.println(SideColorModeNames[sideColorMode]);
        Serial.print(F("Side Rate: "));
        Serial.println(sideBlinkRate);
        Serial.print(F("Block Rate: "));
        Serial.println(blockBlinkRate);
        printBlockColors();
    }
    
    Serial.println(F("\n--- General Settings ---"));
    Serial.print(F("Global Brightness: "));
    Serial.println(ledBrightness);
    Serial.print(F("Body Brightness: "));
    Serial.print(bodyBrightness);
    Serial.println(F("%"));
    Serial.print(F("Mouth Outer Boost: "));
    Serial.print(mouthOuterBoost);
    Serial.println(F("%"));
    Serial.print(F("Mouth Inner Boost: "));
    Serial.print(mouthInnerBoost);
    Serial.println(F("%"));
    Serial.print(F("Speed: "));
    Serial.println(effectSpeed);
    Serial.print(F("Fade: "));
    Serial.println(fadeSpeed);
    Serial.println(F("========================\n"));
}

void printBlockColors() {
    Serial.println(F("Block Colors:"));
    const char* boardNames[3] = {"Left", "Middle", "Right"};
    for (int board = 0; board < 3; board++) {
        Serial.print(F("  "));
        Serial.print(boardNames[board]);
        Serial.print(F(": "));
        for (int block = 0; block < 3; block++) {
            int blockIndex = (2 - board) * 3 + block;
            Serial.print(F("B"));
            Serial.print(block + 1);
            Serial.print(F("="));
            Serial.print(ColorNames[blockColors[blockIndex]]);
            if (block < 2) Serial.print(F(", "));
        }
        Serial.println();
    }
}

void processSerialCommand() {
    inputString.trim();
    inputString.toLowerCase();

    if (inputString.length() == 0) {
        stringComplete = false;
        return;
    }

    Serial.print(F("Processing command: '"));
    Serial.print(inputString);
    Serial.println(F("'"));
    
    if (inputString == "help" || inputString == "h") {
        printHelp();
    }
    else if (inputString == "status") {
        printCurrentSettings();
    }
    else if (inputString == "showblocks") {
        printBlockColors();
    }

//Playlist command processing
    else if (inputString == "playlist on") {
        if (playlistSize > 0) {
            playlistActive = true;
            playlistIndex = 0;
            currentPattern = playlist[playlistIndex].pattern;
            playlistPatternStartTime = millis();
            Serial.println(F("Playlist ON."));
            Serial.print(F("Starting with pattern "));
            Serial.println(currentPattern);
        } else {
            Serial.println(F("Playlist is empty. Cannot start."));
        }
    }
    else if (inputString == "playlist off") {
        playlistActive = false;
        Serial.println(F("Playlist OFF."));
    }
    else if (inputString == "playlist show") {
        Serial.println(F("--- Current Playlist ---"));
        if (playlistSize == 0) {
            Serial.println(F("[Empty]"));
        } else {
            for (int i = 0; i < playlistSize; i++) {
                Serial.print(i + 1);
                Serial.print(F(": Pattern "));
                Serial.print(playlist[i].pattern);
                Serial.print(F(" ("));
                Serial.print(patternNames[playlist[i].pattern]);
                Serial.print(F(") for "));
                Serial.print(playlist[i].duration);
                Serial.println(F("s"));
            }
        }
        Serial.println(F("------------------------"));
    }
    else if (inputString.startsWith("playlist ")) {
        playlistActive = false; // Stop playlist while redefining it
        parsePlaylistCommand(inputString);
    }

    // Pattern commands
    else if (inputString.startsWith("s ")) {
        int pattern = inputString.substring(2).toInt();
        if (pattern >= 0 && pattern < NUM_PATTERNS) {
            requestedPattern = pattern; // Set request instead of changing directly
            demoMode = false;
            playlistActive = false;
            Serial.print(F("Pattern change requested to: "));
            Serial.println(patternNames[pattern]);
        } else {
            Serial.print(F("Invalid pattern! Use 0-"));
            Serial.println(NUM_PATTERNS - 1);
        }
    }
    else if (inputString == "next") {
        uint8_t nextPat = (currentPattern + 1) % NUM_PATTERNS;
        requestedPattern = nextPat; // Set request
        demoMode = false;
        playlistActive = false;
        Serial.print(F("Pattern change requested to: "));
        Serial.println(patternNames[nextPat]);
    }
    else if (inputString == "prev") {
        uint8_t prevPat = (currentPattern == 0) ? NUM_PATTERNS - 1 : currentPattern - 1;
        requestedPattern = prevPat; // Set request
        demoMode = false;
        playlistActive = false;
        Serial.print(F("Pattern change requested to: "));
        Serial.println(patternNames[prevPat]);
    }
    // Eye flicker commands
    else if (inputString == "eyeflicker on") {
        eyeFlickerEnabled = true;
        Serial.println(F("Eye flicker enabled"));
    }
    else if (inputString == "eyeflicker off") {
        eyeFlickerEnabled = false;
        Serial.println(F("Eye flicker disabled - eyes now static"));
    }
    else if (inputString == "eyeflicker settings") {
        printEyeFlickerSettings();
    }
    else if (inputString.startsWith("eyeflickertime ")) {
        int spaceIndex = inputString.indexOf(' ', 15);
        if (spaceIndex > 0) {
            int minTime = inputString.substring(15, spaceIndex).toInt();
            int maxTime = inputString.substring(spaceIndex + 1).toInt();
            if (minTime >= 50 && maxTime > minTime && maxTime <= 5000) {
                eyeFlickerMinTime = minTime;
                eyeFlickerMaxTime = maxTime;
                Serial.print(F("Eye flicker timing: "));
                Serial.print(minTime);
                Serial.print(F("-"));
                Serial.print(maxTime);
                Serial.println(F("ms"));
                
                for (int i = 0; i < NUM_EYES; i++) {
                    EyesIntervalTime[i] = random(eyeFlickerMinTime, eyeFlickerMaxTime);
                }
            } else {
                Serial.println(F("Invalid timing! Use: eyeflickertime <50-5000> <50-5000> (max > min)"));
            }
        } else {
            Serial.println(F("Usage: eyeflickertime <min_ms> <max_ms>"));
        }
    }
    else if (inputString.startsWith("eyestaticbright ")) {
        int brightness = inputString.substring(16).toInt();
        if (brightness >= 0 && brightness <= 255) {
            eyeStaticBrightness = brightness;
            Serial.print(F("Eye static brightness: "));
            Serial.print(brightness);
            Serial.println(F("/255"));
        } else {
            Serial.println(F("Invalid brightness! Use 0-255"));
        }
    }
    // --- NEW EYE COMMANDS ---
    else if (inputString.startsWith("eyecolor2 ")) {
        int color = inputString.substring(10).toInt();
        if (color >= 0 && color < NUM_STANDARD_COLORS) {
            eyeColorIndex2 = color;
            Serial.print(F("Eye color 2 set to: "));
            Serial.println(ColorNames[color]);
        } else {
            Serial.print(F("Invalid color! Use 0-"));
            Serial.println(NUM_STANDARD_COLORS - 1);
        }
    }
    else if (inputString.startsWith("eyemode ")) {
        int mode = inputString.substring(8).toInt();
        if (mode >= 0 && mode < 3) {
            eyeMode = mode;
            Serial.print(F("Eye mode set to: "));
            Serial.print(mode);
            Serial.print(F(" ("));
            Serial.print(EyeModeNames[mode]);
            Serial.println(F(")"));
        } else {
            Serial.println(F("Invalid eye mode! Use 0-2"));
        }
    }
    // Audio commands
    else if (inputString.startsWith("audiomode ")) {
        int mode = inputString.substring(10).toInt();
        if (mode >= 0 && mode <= 4) {
            audioMode = mode;
            Serial.print(F("Audio mode: "));
            Serial.println(AudioModeNames[mode]);
        } else {
            Serial.println(F("Invalid audio mode! Use 0-4"));
        }
    }
    else if (inputString.startsWith("audiosens ")) {
        int sens = inputString.substring(10).toInt();
        if (sens >= 1 && sens <= 10) {
            audioSensitivity = sens;
            Serial.print(F("Audio sensitivity: "));
            Serial.println(sens);
        } else {
            Serial.println(F("Invalid sensitivity! Use 1-10"));
        }
    }
    else if (inputString == "autogain on") {
        audioAutoGain = true;
        Serial.println(F("Auto gain enabled"));
    }
    else if (inputString == "autogain off") {
        audioAutoGain = false;
        Serial.println(F("Auto gain disabled"));
    }
    // Mouth commands
    else if (inputString.startsWith("mouth ")) {
        int pattern = inputString.substring(6).toInt();
        if (pattern >= 0 && pattern < NUM_MOUTH_PATTERNS) {
            mouthPattern = pattern;
            Serial.print(F("Mouth pattern: "));
            Serial.println(MouthPatternNames[pattern]);
        } else {
            Serial.print(F("Invalid mouth pattern! Use 0-"));
            Serial.println(NUM_MOUTH_PATTERNS - 1);
        }
    }
    else if (inputString.startsWith("mouthcolor ")) {
        int color = inputString.substring(11).toInt();
        if (color >= 0 && color < NUM_STANDARD_COLORS) {
            mouthColorIndex = color;
            Serial.print(F("Mouth color: "));
            Serial.println(ColorNames[color]);
        } else {
            Serial.print(F("Invalid mouth color! Use 0-"));
            Serial.println(NUM_STANDARD_COLORS - 1);
        }
    }
    // --- NEW MOUTH COMMANDS ---
    else if (inputString.startsWith("mouthcolor2 ")) {
        int color = inputString.substring(12).toInt();
        if (color >= 0 && color < NUM_STANDARD_COLORS) {
            mouthColorIndex2 = color;
            Serial.print(F("Mouth color 2: "));
            Serial.println(ColorNames[color]);
        } else {
            Serial.print(F("Invalid mouth color! Use 0-"));
            Serial.println(NUM_STANDARD_COLORS - 1);
        }
    }
    else if (inputString.startsWith("mouthsplit ")) {
        int mode = inputString.substring(11).toInt();
        if (mode >= 0 && mode < 5) {
            mouthSplitMode = mode;
            Serial.print(F("Mouth split mode: "));
            Serial.println(MouthSplitNames[mode]);
        } else {
            Serial.println(F("Invalid split mode! Use 0-4"));
        }
    }
    else if (inputString.startsWith("wavespeed ")) {
        int speed = inputString.substring(10).toInt();
        if (speed >= 1 && speed <= 10) {
            waveSpeed = speed;
            Serial.print(F("Wave speed: "));
            Serial.println(speed);
        } else {
            Serial.println(F("Invalid speed! Use 1-10"));
        }
    }
    else if (inputString.startsWith("pulsespeed ")) {
        int speed = inputString.substring(11).toInt();
        if (speed >= 1 && speed <= 10) {
            pulseSpeed = speed;
            Serial.print(F("Pulse speed: "));
            Serial.println(speed);
        } else {
            Serial.println(F("Invalid speed! Use 1-10"));
        }
    }
    // --- END NEW MOUTH COMMANDS ---
    else if (inputString.startsWith("mouthbrightness ")) {
        int bright = inputString.substring(16).toInt();
        if (bright >= 1 && bright <= 255) {
            mouthBrightness = bright;
            Serial.print(F("Mouth brightness: "));
            Serial.println(bright);
        } else {
            Serial.println(F("Invalid brightness! Use 1-255"));
        }
    }
    else if (inputString == "mouthenable on") {
        mouthEnabled = true;
        Serial.println(F("Mouth enabled"));
    }
    else if (inputString == "mouthenable off") {
        mouthEnabled = false;
        Serial.println(F("Mouth disabled"));
    }
    else if (inputString.startsWith("talkspeed ")) {
        int speed = inputString.substring(10).toInt();
        if (speed >= 1 && speed <= 10) {
            talkSpeed = speed;
            Serial.print(F("Talk speed: "));
            Serial.println(speed);
        } else {
            Serial.println(F("Invalid talk speed! Use 1-10"));
        }
    }
    else if (inputString.startsWith("smilewidth ")) {
        int width = inputString.substring(11).toInt();
        if (width >= 2 && width <= 10) {
            smileWidth = width;
            Serial.print(F("Smile width: "));
            Serial.println(width);
        } else {
            Serial.println(F("Invalid smile width! Use 2-10"));
        }
    }
    // Block commands
    else if (inputString.startsWith("blockcolor ")) {
        int spaceIndex = inputString.indexOf(' ', 11);
        if (spaceIndex > 0) {
            int blockIndex = inputString.substring(11, spaceIndex).toInt();
            int colorIndex = inputString.substring(spaceIndex + 1).toInt();
            if (blockIndex >= 0 && blockIndex <= 8 && colorIndex >= 0 && colorIndex < NUM_STANDARD_COLORS) {
                blockColors[blockIndex] = colorIndex;
                demoMode = false;
                const char* boardNames[3] = {"Left", "Middle", "Right"};
                int boardIdx = blockIndex / 3;
                Serial.print(F("Block "));
                Serial.print(blockIndex);
                Serial.print(F(" ("));
                Serial.print(boardNames[boardIdx]);
                Serial.print(F(" B"));
                Serial.print((blockIndex % 3) + 1);
                Serial.print(F(") = "));
                Serial.println(ColorNames[colorIndex]);
            } else {
                Serial.print(F("Invalid! Use: blockcolor <0-8> <0-"));
                Serial.print(NUM_STANDARD_COLORS - 1);
                Serial.println(F(">"));
            }
        }
    }
    else if (inputString.startsWith("blockrate ")) {
        int rate = inputString.substring(10).toInt();
        if (rate >= 1 && rate <= 255) {
            blockBlinkRate = rate;
            Serial.print(F("Block rate: "));
            Serial.println(rate);
        }
    }
    else if (inputString.startsWith("sidecolors ")) {
        int space1 = inputString.indexOf(' ', 11);
        int space2 = inputString.indexOf(' ', space1 + 1);
        if (space1 > 0 && space2 > 0) {
            int c1 = inputString.substring(11, space1).toInt();
            int c2 = inputString.substring(space1 + 1, space2).toInt();
            int c3 = inputString.substring(space2 + 1).toInt();
            if (c1 >= 0 && c1 < NUM_STANDARD_COLORS && 
                c2 >= 0 && c2 < NUM_STANDARD_COLORS && 
                c3 >= 0 && c3 < NUM_STANDARD_COLORS) {
                sideColor1 = c1;
                sideColor2 = c2;
                sideColor3 = c3;
                demoMode = false;
                Serial.print(F("Side colors: "));
                Serial.print(ColorNames[c1]);
                Serial.print(F(", "));
                Serial.print(ColorNames[c2]);
                Serial.print(F(", "));
                Serial.println(ColorNames[c3]);
            } else {
                Serial.print(F("Invalid! Use: sidecolors <0-"));
                Serial.print(NUM_STANDARD_COLORS - 1);
                Serial.print(F("> <0-"));
                Serial.print(NUM_STANDARD_COLORS - 1);
                Serial.print(F("> <0-"));
                Serial.print(NUM_STANDARD_COLORS - 1);
                Serial.println(F(">"));
            }
        }
    }
    else if (inputString.startsWith("sidemode ")) {
        int mode = inputString.substring(9).toInt();
        if (mode >= 0 && mode <= 4) {
            sideColorMode = mode;
            sideColorCycleIndex = 0;
            Serial.print(F("Side mode: "));
            Serial.println(SideColorModeNames[mode]);
        }
    }
    else if (inputString.startsWith("siderate ")) {
        int rate = inputString.substring(9).toInt();
        if (rate >= 1 && rate <= 255) {
            sideBlinkRate = rate;
            Serial.print(F("Side rate: "));
            Serial.println(rate);
        }
    }
    // Pattern-specific colors
    else if (inputString.startsWith("color ")) {
        int color = inputString.substring(6).toInt();
        if (color >= 0 && color < NUM_STANDARD_COLORS) {
            solidColorIndex = color;
            demoMode = false;
            Serial.print(F("Solid color: "));
            Serial.println(ColorNames[color]);
        } else {
            Serial.print(F("Invalid color! Use 0-"));
            Serial.println(NUM_STANDARD_COLORS - 1);
        }
    }
    else if (inputString.startsWith("solidmode ")) {
        int mode = inputString.substring(10).toInt();
        if (mode >= 0 && mode <= 1) {
            solidMode = mode;
            Serial.print(F("Solid mode: "));
            Serial.println(mode == 0 ? "Static" : "Blink");
        }
    }
    else if (inputString.startsWith("flashcolor ")) {
        int color = inputString.substring(11).toInt();
        if (color >= 0 && color < NUM_STANDARD_COLORS) {
            flashColorIndex = color;
            Serial.print(F("Flash color: "));
            Serial.println(ColorNames[color]);
        } else {
            Serial.print(F("Invalid flash color! Use 0-"));
            Serial.println(NUM_STANDARD_COLORS - 1);
        }
    }
    else if (inputString.startsWith("flashspeed ")) {
        int speed = inputString.substring(11).toInt();
        if (speed >= 1 && speed <= 10) {
            flashSpeed = speed;
            Serial.print(F("Flash speed: "));
            Serial.println(speed);
        }
    }
    else if (inputString.startsWith("shortcolor ")) {
        int color = inputString.substring(11).toInt();
        if (color >= 0 && color < NUM_STANDARD_COLORS) {
            shortColorIndex = color;
            Serial.print(F("Short color: "));
            Serial.println(ColorNames[color]);
        } else {
            Serial.print(F("Invalid short color! Use 0-"));
            Serial.println(NUM_STANDARD_COLORS - 1);
        }
    }
    else if (inputString.startsWith("knightcolor ")) {
        int color = inputString.substring(12).toInt();
        if (color >= 0 && color < NUM_STANDARD_COLORS) {
            knightColorIndex = color;
            Serial.print(F("Knight color: "));
            Serial.println(ColorNames[color]);
        } else {
            Serial.print(F("Invalid knight color! Use 0-"));
            Serial.println(NUM_STANDARD_COLORS - 1);
        }
    }
    else if (inputString.startsWith("breathcolor ")) {
        int color = inputString.substring(12).toInt();
        if (color >= 0 && color < NUM_STANDARD_COLORS) {
            breathingColorIndex = color;
            Serial.print(F("Breath color: "));
            Serial.println(ColorNames[color]);
        } else {
            Serial.print(F("Invalid breath color! Use 0-"));
            Serial.println(NUM_STANDARD_COLORS - 1);
        }
    }
    else if (inputString.startsWith("matrixcolor ")) {
        int color = inputString.substring(12).toInt();
        if (color >= 0 && color < NUM_STANDARD_COLORS) {
            matrixColorIndex = color;
            Serial.print(F("Matrix color: "));
            Serial.println(ColorNames[color]);
        } else {
            Serial.print(F("Invalid matrix color! Use 0-"));
            Serial.println(NUM_STANDARD_COLORS - 1);
        }
    }
    else if (inputString.startsWith("strobecolor ")) {
        int color = inputString.substring(12).toInt();
        if (color >= 0 && color < NUM_STANDARD_COLORS) {
            strobeColorIndex = color;
            Serial.print(F("Strobe color: "));
            Serial.println(ColorNames[color]);
        } else {
            Serial.print(F("Invalid strobe color! Use 0-"));
            Serial.println(NUM_STANDARD_COLORS - 1);
        }
    }
    else if (inputString.startsWith("confetti ")) {
        int spaceIndex = inputString.indexOf(' ', 9);
        if (spaceIndex > 0) {
            int c1 = inputString.substring(9, spaceIndex).toInt();
            int c2 = inputString.substring(spaceIndex + 1).toInt();
            if (c1 >= 0 && c1 < NUM_STANDARD_COLORS && c2 >= 0 && c2 < NUM_STANDARD_COLORS) {
                confettiColor1 = c1;
                confettiColor2 = c2;
                demoMode = false;
                Serial.print(F("Confetti: "));
                Serial.print(ColorNames[c1]);
                Serial.print(F(" + "));
                Serial.println(ColorNames[c2]);
            } else {
                Serial.print(F("Invalid! Use: confetti <0-"));
                Serial.print(NUM_STANDARD_COLORS - 1);
                Serial.print(F("> <0-"));
                Serial.print(NUM_STANDARD_COLORS - 1);
                Serial.println(F(">"));
            }
        }
    }
else if (inputString.startsWith("eyecolor ")) {
    String colorStr = inputString.substring(9);
    colorStr.trim();
    int color = colorStr.toInt();
    
    if (color >= 0 && color < NUM_STANDARD_COLORS) {
        eyeColorIndex = color;
        Serial.print(F("Eye color set to: "));
        Serial.print(color);
        Serial.print(F(" ("));
        Serial.print(ColorNames[color]);
        Serial.println(F(")"));
    } else {
        Serial.print(F("Invalid eye color! Use 0-"));
        Serial.println(NUM_STANDARD_COLORS - 1);
    }
}
    // Demo mode
    else if (inputString == "demo on") {
        demoMode = true;
        demoPatternIndex = 1;
        demoColorIndex = 0;
        demoStep = 0;
        lastDemoChange = millis();
        currentPattern = demoPatternIndex;
        Serial.print(F("Demo mode ON ("));
        Serial.print(demoTime);
        Serial.println(F("s)"));
    }
    else if (inputString == "demo off") {
        demoMode = false;
        Serial.println(F("Demo mode OFF"));
    }
    else if (inputString.startsWith("demotime ")) {
        int time = inputString.substring(9).toInt();
        if (time >= 5 && time <= 300) {
            demoTime = time;
            Serial.print(F("Demo time: "));
            Serial.print(time);
            Serial.println(F("s"));
        }
    }
    // General settings
    else if (inputString.startsWith("brightness ")) {
        int bright = inputString.substring(11).toInt();
        if (bright >= 1 && bright <= 255) {
            ledBrightness = bright;
            FastLED.setBrightness(ledBrightness);
            Serial.print(F("Brightness: "));
            Serial.println(bright);
        }
    }
    else if (inputString.startsWith("eyebrightness ")) {
        int bright = inputString.substring(14).toInt();
        if (bright >= 50 && bright <= 200) {
            eyeBrightness = bright;
            Serial.print(F("Eye brightness: "));
            Serial.print(bright);
            Serial.println(F("%"));
        } else {
            Serial.println(F("Invalid! Use 50-200%"));
        }
    }
    else if (inputString.startsWith("bodybrightness ")) {
        int bright = inputString.substring(15).toInt();
        if (bright >= 50 && bright <= 200) {
            bodyBrightness = bright;
            Serial.print(F("Body brightness: "));
            Serial.print(bright);
            Serial.println(F("%"));
        } else {
            Serial.println(F("Invalid! Use 50-200%"));
        }
    }
    else if (inputString.startsWith("mouthouter ")) {
        int boost = inputString.substring(11).toInt();
        if (boost >= 50 && boost <= 200) {
            mouthOuterBoost = boost;
            Serial.print(F("Mouth outer boost: "));
            Serial.print(boost);
            Serial.println(F("%"));
        } else {
            Serial.println(F("Invalid! Use 50-200%"));
        }
    }
    else if (inputString.startsWith("mouthinner ")) {
        int boost = inputString.substring(11).toInt();
        if (boost >= 50 && boost <= 200) {
            mouthInnerBoost = boost;
            Serial.print(F("Mouth inner boost: "));
            Serial.print(boost);
            Serial.println(F("%"));
        } else {
            Serial.println(F("Invalid! Use 50-200%"));
        }
    }
    else if (inputString.startsWith("speed ")) {
        int speed = inputString.substring(6).toInt();
        if (speed >= 1 && speed <= 255) {
            effectSpeed = speed;
            Serial.print(F("Speed: "));
            Serial.println(speed);
        }
    }
    else if (inputString.startsWith("fade ")) {
        int fade = inputString.substring(5).toInt();
        if (fade >= 1 && fade <= 50) {
            fadeSpeed = fade;
            Serial.print(F("Fade: "));
            Serial.println(fade);
        }
    }
    else if (inputString.startsWith("sidetime ")) {
        int spaceIndex = inputString.indexOf(' ', 9);
        if (spaceIndex > 0) {
            int minT = inputString.substring(9, spaceIndex).toInt();
            int maxT = inputString.substring(spaceIndex + 1).toInt();
            if (minT > 0 && maxT > minT && maxT <= 10000) {
                sideMinTime = minT;
                sideMaxTime = maxT;
                Serial.print(F("Side time: "));
                Serial.print(minT);
                Serial.print(F("-"));
                Serial.print(maxT);
                Serial.println(F("ms"));
            }
        }
    }
    else if (inputString.startsWith("blocktime ")) {
        int spaceIndex = inputString.indexOf(' ', 10);
        if (spaceIndex > 0) {
            int minT = inputString.substring(10, spaceIndex).toInt();
            int maxT = inputString.substring(spaceIndex + 1).toInt();
            if (minT > 0 && maxT > minT && maxT <= 10000) {
                blockMinTime = minT;
                blockMaxTime = maxT;
                Serial.print(F("Block time: "));
                Serial.print(minT);
                Serial.print(F("-"));
                Serial.print(maxT);
                Serial.println(F("ms"));
            }
        }
    }
    else if (inputString.startsWith("audiothreshold ")) {
        int thresh = inputString.substring(15).toInt();
        if (thresh >= 50 && thresh <= 500) {
            audioThreshold = thresh;
            Serial.print(F("Audio threshold: "));
            Serial.println(thresh);
        }
    }
    // System commands
    else if (inputString == "save") {
        saveSettings();
    }
    else if (inputString == "load") {
        loadSettings();
        FastLED.setBrightness(ledBrightness);
        printCurrentSettings();
    }
    else if (inputString == "reset") {
        resetToDefaults();
        FastLED.setBrightness(ledBrightness);
        printCurrentSettings();
    }
    // User preset commands
    else if (inputString.startsWith("saveuser ")) {
        int preset = inputString.substring(9).toInt();
        if (preset >= 1 && preset <= 3) {
            saveUserPreset(preset);
        } else {
            Serial.println(F("Invalid preset! Use 1-3"));
        }
    }
    else if (inputString.startsWith("loaduser ")) {
        int preset = inputString.substring(9).toInt();
        if (preset >= 1 && preset <= 3) {
            if (loadUserPreset(preset)) {
                printCurrentSettings();
            }
        } else {
            Serial.println(F("Invalid preset! Use 1-3"));
        }
    }
    else if (inputString.startsWith("deleteuser ")) {
        int preset = inputString.substring(11).toInt();
        if (preset >= 1 && preset <= 3) {
            deleteUserPreset(preset);
        } else {
            Serial.println(F("Invalid preset! Use 1-3"));
        }
    }
    else if (inputString == "listpresets") {
        Serial.println(F("\n=== User Presets ==="));
        for (int i = 1; i <= 3; i++) {
            String presetKey = "preset" + String(i);
            if (preferences.getBool((presetKey + "_saved").c_str(), false)) {
                uint8_t pattern = preferences.getUChar((presetKey + "_pattern").c_str(), 0);
                Serial.print(F("Preset "));
                Serial.print(i);
                Serial.print(F(": "));
                Serial.print(patternNames[pattern]);
                Serial.print(F(" (Pattern "));
                Serial.print(pattern);
                Serial.println(F(")"));
            } else {
                Serial.print(F("Preset "));
                Serial.print(i);
                Serial.println(F(": [Empty]"));
            }
        }
        Serial.println(F("===================\n"));
    }
    // =====================================================
    // v5.0 NEW COMMANDS
    // =====================================================
    // v5.0: Extended Preset Manager (10 slots)
    else if (inputString.startsWith("preset save ")) {
        String args = inputString.substring(12);
        int spacePos = args.indexOf(' ');
        int slot;
        String name = "";
        if (spacePos > 0) {
            slot = args.substring(0, spacePos).toInt();
            name = args.substring(spacePos + 1);
        } else {
            slot = args.toInt();
        }
        if (slot >= 1 && slot <= MAX_PRESETS) {
            presetManager.savePreset(slot - 1, name.length() > 0 ? name.c_str() : nullptr);
        } else {
            Serial.print(F("Invalid slot! Use 1-"));
            Serial.println(MAX_PRESETS);
        }
    }
    else if (inputString.startsWith("preset load ")) {
        int slot = inputString.substring(12).toInt();
        if (slot >= 1 && slot <= MAX_PRESETS) {
            presetManager.loadPreset(slot - 1);
        } else {
            Serial.print(F("Invalid slot! Use 1-"));
            Serial.println(MAX_PRESETS);
        }
    }
    else if (inputString.startsWith("preset delete ")) {
        int slot = inputString.substring(14).toInt();
        if (slot >= 1 && slot <= MAX_PRESETS) {
            presetManager.deletePreset(slot - 1);
        } else {
            Serial.print(F("Invalid slot! Use 1-"));
            Serial.println(MAX_PRESETS);
        }
    }
    else if (inputString == "preset list") {
        presetManager.listPresets();
    }
    // v5.0: System Monitoring
    else if (inputString == "sysinfo") {
        systemMonitor.printStatus();
    }
    // v5.0: Event Logger
    else if (inputString == "eventlog") {
        eventLogger.printLog();
    }
    else if (inputString == "eventlog clear") {
        eventLogger.clear();
        Serial.println(F("Event log cleared"));
    }
    // =====================================================
    else {
        Serial.print(F("Unknown: "));
        Serial.println(inputString);
        Serial.println(F("Type 'help' for commands"));
    }
    
    inputString = "";
    stringComplete = false;

    while (Serial.available()) {
        Serial.read();
    }
}