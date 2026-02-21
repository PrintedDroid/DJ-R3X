# DJ-R3X LED Controller v5.1.0 "Line-In Edition"
**Advanced ESP32-C3/S3 based LED controller for Star Wars DJ-R3X (Rex) animatronic builds**

**Universal Firmware** - Automatically detects and configures for ESP32-C3 Mini OR ESP32-S3 Mini

---

## Overview

This firmware brings your DJ-R3X (Rex from Star Wars: Galaxy's Edge) animatronic to life with stunning LED animations, audio reactivity, and professional control features. Designed for Printed-Droid builders who demand exceptional results.

**NEW in v5.1:** Line-In audio input support! Connect directly to a mixer, audio interface, or media player for clean, noise-free audio-reactive lighting. Switchable via serial menu between Microphone and Line-In modes.

### Key Features

- **60 Body LEDs** - 3 panels with 20 WS2812B LEDs each (8 side LEDs + 3×4 block LEDs per panel)
- **2 Eye LEDs** - Independent eye control with flicker effects and dual-color modes
- **80 Mouth LEDs** - 12-row LED matrix for expressive mouth animations
- **20 Body Patterns** - Including Plasma, Fire, Twinkle, Rainbow, Matrix Rain, and more
- **15 Mouth Patterns** - Talk, Smile, Audio Reactive, Heartbeat, Spectrum Analyzer, and more
- **Audio Reactivity** - Real-time sound response with auto-gain and multiple routing modes
- **FreeRTOS Multi-threading** - Dedicated audio task on Core 0 (ESP32-S3 only, auto-enabled)
- **System Monitoring** - Health checks, memory monitoring, and auto-recovery
- **10 Preset Slots** - Save and recall complete configurations
- **Playlist System** - Automated pattern sequences with smooth crossfade transitions
- **Event Logging** - Track system events for debugging and monitoring
- **Animated Boot Sequence** - Professional startup animation with LED test

---

## Hardware Requirements

### Core Components

| Component | Specification | Notes |
|-----------|---------------|-------|
| **Controller** | LOLIN C3 Mini (ESP32-C3) **OR**<br>LOLIN S3 Mini (ESP32-S3) | 4MB Flash, WiFi/BLE capable<br>**Auto-detected by firmware** |
| **Body LEDs** | WS2812B strips | 3 panels × 20 LEDs = 60 total |
| **Eye LEDs** | WS2812B | 2 LEDs, daisy-chained with mouth |
| **Mouth LEDs** | WS2812B matrix | 80 LEDs in 12-row configuration |
| **Microphone** | Analog microphone module | For audio reactivity |
| **Power Supply** | 5V 5A minimum | Calculate: (142 LEDs × 60mA) + controller |

### Board Comparison

| Feature | ESP32-C3 Mini | ESP32-S3 Mini |
|---------|---------------|---------------|
| **CPU Cores** | 1 (Single-Core) | 2 (Dual-Core) |
| **FreeRTOS Audio Task** | Disabled (prevents flicker) | **Enabled** (better audio) |
| **LED Pins** | 3, 4, 5, 6 | 5, 6, 7, 8 |
| **Performance** | Good | **Excellent** |
| **Recommended For** | Budget builds | **Best performance** |

### LED Panel Layout

Each body panel contains:
```
┌─────────────────────────────────┐
│  Side LEDs (8)                  │
│  ○ ○ ○ ○ ○ ○ ○ ○                │
├─────────────────────────────────┤
│  Block 1    Block 2    Block 3  │
│  ○ ○        ○ ○        ○ ○      │
│  ○ ○        ○ ○        ○ ○      │
└─────────────────────────────────┘
```

### Mouth LED Matrix Layout

```
Row 0:  ○ ○ ○ ○ ○ ○ ○ ○  (8 LEDs)
Row 1:  ○ ○ ○ ○ ○ ○ ○ ○  (8 LEDs)
Row 2:  ○ ○ ○ ○ ○ ○ ○ ○  (8 LEDs)
Row 3:  ○ ○ ○ ○ ○ ○ ○ ○  (8 LEDs)
Row 4:  ○ ○ ○ ○ ○ ○ ○ ○  (8 LEDs)
Row 5:  ○ ○ ○ ○ ○ ○ ○ ○  (8 LEDs)  <- Center
Row 6:  ○ ○ ○ ○ ○ ○ ○ ○  (8 LEDs)
Row 7:  ○ ○ ○ ○ ○ ○ ○ ○  (8 LEDs)
Row 8:    ○ ○ ○ ○ ○ ○    (6 LEDs)
Row 9:      ○ ○ ○ ○      (4 LEDs)
Row 10:     ○ ○ ○ ○      (4 LEDs)
Row 11:       ○ ○        (2 LEDs)
                         ─────────
                         80 LEDs total
```

---

## Pin Configuration

### ESP32-C3 Mini Pinout

| GPIO Pin | Function | Description |
|----------|----------|-------------|
| **Pin 3** | LED_PIN_RIGHT | Right body panel (20 LEDs) |
| **Pin 4** | LED_PIN_MIDDLE | Middle body panel (20 LEDs) |
| **Pin 5** | LED_PIN_LEFT | Left body panel (20 LEDs) |
| **Pin 6** | EYES_MOUTH_PIN | Eyes + Mouth daisy-chained (2 + 80 LEDs) |
| **Pin 1** | MIC_PIN | Analog microphone input |

### ESP32-S3 Mini Pinout

| GPIO Pin | Function | Description |
|----------|----------|-------------|
| **Pin 5** | LED_PIN_RIGHT | Right body panel (20 LEDs) |
| **Pin 6** | LED_PIN_MIDDLE | Middle body panel (20 LEDs) |
| **Pin 7** | LED_PIN_LEFT | Left body panel (20 LEDs) |
| **Pin 8** | EYES_MOUTH_PIN | Eyes + Mouth daisy-chained (2 + 80 LEDs) |
| **Pin 1** | MIC_PIN | Analog microphone input |

### Wiring Diagrams

**ESP32-C3 Mini:**
```
ESP32-C3 Mini          LED Strips
┌──────────────┐
│          GP3 ├──────► Right Panel (20 LEDs)
│          GP4 ├──────► Middle Panel (20 LEDs)
│          GP5 ├──────► Left Panel (20 LEDs)
│          GP6 ├──────► Eyes (2) ──► Mouth (80)
│          GP1 ├──────► Microphone (Analog)
│          5V  ├──────► LED VCC (5V)
│          GND ├──────► LED GND + Mic GND
└──────────────┘
```

**ESP32-S3 Mini:**
```
ESP32-S3 Mini          LED Strips
┌──────────────┐
│          GP5 ├──────► Right Panel (20 LEDs)
│          GP6 ├──────► Middle Panel (20 LEDs)
│          GP7 ├──────► Left Panel (20 LEDs)
│          GP8 ├──────► Eyes (2) ──► Mouth (80)
│          GP1 ├──────► Microphone (Analog)
│          5V  ├──────► LED VCC (5V)
│          GND ├──────► LED GND + Mic GND
└──────────────┘
```

**Important Notes:**
- **Board-specific pins:** C3 uses pins 3,4,5,6 / S3 uses pins 5,6,7,8 (auto-configured)
- Eyes and mouth are daisy-chained on a single data line (Pin 6 on C3, Pin 8 on S3)
- Eyes come first (LEDs 0-1), then mouth (LEDs 2-81)
- Use adequate power injection for 142 LEDs
- Add 300-500Ω resistor on data lines for signal integrity
- Add 1000µF capacitor near LED power input

---

## Installation

### Prerequisites

- **Arduino IDE 2.0+** or **PlatformIO**
- **ESP32 Board Support** (esp32 by Espressif v2.0+)
- **FastLED Library v3.9.0** (critical - must be this version or newer)

### Arduino IDE Setup

1. **Install ESP32 Board Support:**
   - File → Preferences → Additional Board Manager URLs
   - Add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools → Board → Board Manager → Search "ESP32" → Install

2. **Install Required Libraries:**
   - Sketch → Include Library → Manage Libraries
   - Search and install:
     - **FastLED** (v3.9.0 or newer)
     - **Preferences** (built-in)

3. **Board Configuration:**

   **For ESP32-C3 Mini:**
   | Setting | Value |
   |---------|-------|
   | Board | LOLIN C3 Mini |
   | USB CDC On Boot | Enabled |
   | CPU Frequency | 160MHz |
   | Flash Size | 4MB |
   | Partition Scheme | Default |
   | Upload Speed | 921600 |

   **For ESP32-S3 Mini:**
   | Setting | Value |
   |---------|-------|
   | Board | LOLIN S3 Mini |
   | USB CDC On Boot | Enabled |
   | CPU Frequency | 240MHz |
   | Flash Size | 4MB |
   | Partition Scheme | Default 4MB with spiffs |
   | Upload Speed | 921600 |
   | USB Mode | Hardware CDC and JTAG |

4. **Upload:**
   - Connect your ESP32 board via USB
   - Select correct COM port
   - Click Upload
   - **The firmware automatically detects your board type on first boot!**

### PlatformIO Setup

Create `platformio.ini` for your board:

**For ESP32-C3 Mini:**
```ini
[env:lolin_c3_mini]
platform = espressif32
board = lolin_c3_mini
framework = arduino
monitor_speed = 115200
lib_deps =
    fastled/FastLED@^3.9.0
build_flags =
    -DARDUINO_USB_CDC_ON_BOOT=1
```

**For ESP32-S3 Mini:**
```ini
[env:lolin_s3_mini]
platform = espressif32
board = lolin_s3_mini
framework = arduino
monitor_speed = 115200
lib_deps =
    fastled/FastLED@^3.9.0
build_flags =
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DARDUINO_USB_MODE=1
```

---

## Body Patterns (20 Total)

| # | Pattern | Description | Audio Reactive |
|---|---------|-------------|----------------|
| 0 | LEDs Off | All LEDs fade to black | No |
| 1 | Random Blocks | Random block/side activation with configurable colors | No |
| 2 | Solid Color | Static single color on all LEDs | No |
| 3 | Short Circuit | Sparking/electrical effect | No |
| 4 | Confetti Red/White | Random red and white sparkles | No |
| 5 | Rainbow | Classic rainbow cycle | No |
| 6 | Rainbow with Glitter | Rainbow with random white sparkles | No |
| 7 | Confetti | Multi-color random sparkles | No |
| 8 | Juggle | Multiple colored dots bouncing | No |
| 9 | Audio Sync | Color changes with sound | **Yes** |
| 10 | Solid Flash | Alternating on/off flash | No |
| 11 | Knight Rider | KITT-style scanning light | No |
| 12 | Breathing | Smooth brightness fade in/out | No |
| 13 | Matrix Rain | Digital rain falling effect | No |
| 14 | Strobe | Fast strobe flash | No |
| 15 | Audio VU Meter | Volume level visualization | **Yes** |
| 16 | Custom Block Sequence | User-configurable block colors | No |
| 17 | **Plasma** ⭐ | Flowing plasma waves with color shifting | No |
| 18 | **Fire** ⭐ | Realistic fire simulation | No |
| 19 | **Twinkle** ⭐ | Random twinkling star effect | No |

⭐ = New in v5.0

---

## Mouth Patterns (15 Total)

| # | Pattern | Description | Audio Reactive |
|---|---------|-------------|----------------|
| 0 | Off | All mouth LEDs off | No |
| 1 | Talk | Animated talking mouth | No |
| 2 | Smile | Static smile shape | No |
| 3 | Audio Reactive | Mouth opens with sound | **Yes** |
| 4 | Rainbow | Rainbow color cycle | No |
| 5 | Debug | LED position testing | No |
| 6 | Wave | Flowing wave animation | No |
| 7 | Pulse | Breathing/pulsing effect | No |
| 8 | VU Meter Horizontal | Horizontal audio bars | **Yes** |
| 9 | VU Meter Vertical | Vertical audio bars | **Yes** |
| 10 | Frown | Inverted smile (sad face) | No |
| 11 | Sparkle | Random sparkle effect | No |
| 12 | **Matrix** ⭐ | Matrix-style falling drops | No |
| 13 | **Heartbeat** ⭐ | Double-pulse heartbeat animation | No |
| 14 | **Spectrum** ⭐ | Audio spectrum analyzer | **Yes** |

⭐ = New in v5.0

---

## Eye Modes

| Mode | Name | Description |
|------|------|-------------|
| 0 | Single Color | Both eyes same color |
| 1 | Dual Color | Each eye different color |
| 2 | Alternating | Eyes alternate between two colors |

### Eye Flicker Effect

The eyes support a natural flicker effect that simulates electrical/robotic behavior:

- **Enable/Disable:** `eyeflicker on` / `eyeflicker off`
- **Timing:** Configurable min/max intervals
- **Static Brightness:** Brightness when flicker is disabled

---

## Audio System

### Audio Modes

| Mode | Name | Description |
|------|------|-------------|
| 0 | Off | No audio reactivity |
| 1 | Mouth Only | Only mouth responds to audio |
| 2 | Body Sides Only | Only side LEDs respond |
| 3 | Body All | All body LEDs respond |
| 4 | Everything | Full audio reactivity |

### Auto-Gain System

The audio system automatically adjusts sensitivity based on ambient sound levels:

- Tracks minimum and maximum audio levels
- Dynamically adjusts threshold
- Decays over time for adaptation
- Configurable sensitivity (1-10)

### Audio Input Modes (v5.1)

| Mode | Name | Command | Description |
|------|------|---------|-------------|
| 0 | Microphone | `audioinput mic` | Default - uses onboard analog microphone |
| 1 | Line-In | `audioinput linein` | Direct audio feed from mixer/player (requires adapter) |

When switching to Line-In, the firmware automatically adjusts sensitivity for the stronger signal level. The setting is saved to flash.

### Technical Specifications

- **Sample Rate:** 200 Hz (5ms interval via FreeRTOS task on S3, or main loop on C3)
- **Resolution:** 12-bit ADC
- **Processing:** Running average with 10-sample window
- **Core Assignment (S3 only):** Audio task runs on Core 0, main loop on Core 1

---

## Audio-to-Light: How It Works

This section explains the complete signal flow from audio input to LED output for users who want to understand or tune the audio-reactive behavior.

### Signal Flow

```
Audio Source (Mic or Line-In)
        |
        v
  [ADC 12-bit Read: 0-4095]
        |
        v
  [DC Offset Removal: abs(reading - calibratedOffset)]
        |
        v
  [10-Sample Running Average]
        |
        v
  [Sensitivity Scaling: map(audio, 0, range, 0, sensitivity*100)]
        |     range = 2048 (Mic) or 4095 (Line-In)
        v
  [Auto-Gain Threshold Adjustment]
        |
        v
  [Audio Mode Routing]
        |
        +---> Body Patterns: audioSync, audioVUMeter
        +---> Mouth Patterns: mouthAudioReactive, mouthVUMeter, mouthSpectrum
        |
        v
  [FastLED.show() -> 142 LEDs]
```

### ADC Calibration

On startup, the firmware samples the ADC input 200 times to determine the DC offset (the "silence" baseline). This calibrated value is used instead of a hardcoded midpoint, which improves accuracy across different hardware setups.

### Audio Processing Pipeline

1. **Raw Read** (`readAudioLevel`): Reads the analog pin every 10ms, subtracts the calibrated DC offset, and takes the absolute value to get the amplitude.

2. **Averaging** (`processAudioLevel`): Maintains a 10-sample circular buffer. The running average smooths out transient spikes for stable VU meter behavior.

3. **Sensitivity Mapping**: The raw amplitude is scaled based on the user-configured sensitivity (1-10) and the input mode. Line-In uses a wider mapping range (4095) than Microphone (2048) because line-level signals have higher amplitude.

4. **Auto-Gain**: When enabled, the system tracks minimum and maximum audio levels over time and dynamically adjusts the threshold to the midpoint. This means the LEDs respond consistently whether the music is quiet or loud.

### Audio-Reactive Body Patterns

| Pattern | Index | Mapping | Behavior |
|---------|-------|---------|----------|
| **Audio Sync** | 9 | `audio -> numLEDs (0-8 side LEDs)` | Side LEDs light up proportionally. Blocks activate at >70% level. White sparkles at >80%. |
| **Audio VU Meter** | 15 | `averageAudio -> vuLevel (0-8)` | Classic green/yellow/red VU bar. White block peak indicators at >80%. |

### Audio-Reactive Mouth Patterns

| Pattern | Index | Mapping | Behavior |
|---------|-------|---------|----------|
| **Audio Reactive** | 3 | `audio -> activeRows (0-12)` | Mouth opens from center outward. Hue shifts from green (quiet) to red (loud). |
| **VU Meter Horiz** | 8 | `audio -> level (0-4 per side)` | Horizontal bars expand from center of each row. |
| **VU Meter Vert** | 9 | `audio -> level (0-12 rows)` | Vertical bar fills rows from bottom to top. |
| **Spectrum** | 14 | `audio + random variation -> 8 bands` | Simulated 8-band spectrum analyzer with green-to-red gradient. |

**Note:** The Spectrum pattern simulates frequency bands by adding small random variation to the single amplitude value. It does not perform actual FFT/frequency analysis. For true frequency separation, an external FFT module would be needed.

### Tuning Tips

- **Sensitivity too high?** LEDs always maxed out → lower with `audiosens 3`
- **Sensitivity too low?** LEDs barely respond → raise with `audiosens 8`
- **Erratic response?** Enable auto-gain: `autogain on`
- **Want manual control?** Disable auto-gain and set threshold: `autogain off` then `audiothreshold 150`
- **Line-In too hot?** The firmware handles this automatically, but you can fine-tune with `audiosens 1-3`

---

## Line-In Audio Input (v5.1)

Instead of using the onboard microphone (which picks up ambient noise), you can feed a clean audio signal directly from a mixer, DJ controller, media player, or audio interface into the ESP32's ADC pin.

### Why Use Line-In?

| | Microphone | Line-In |
|---|-----------|---------|
| **Signal quality** | Picks up ambient noise | Clean, direct signal |
| **Consistency** | Varies with distance/placement | Consistent level |
| **Setup** | Plug and play | Requires adapter circuit |
| **Best for** | Casual use, demos | DJ setups, events, permanent installations |

### Required Hardware Adapter

Line-level audio signals are AC (they swing positive and negative around 0V, typically ±1V for consumer gear). The ESP32 ADC can only read **0-3.3V** - negative voltages will damage it. A simple bias circuit shifts the signal into the safe range.

**Components needed (< $1):**

| Component | Value | Purpose |
|-----------|-------|---------|
| Coupling capacitor | 10µF electrolytic | Blocks DC from source device |
| Bias resistor 1 | 100kΩ | Forms voltage divider |
| Bias resistor 2 | 100kΩ | Forms voltage divider |
| Protection resistor (optional) | 10kΩ | Limits current to ADC |

**Schematic:**

```
                       3.3V
                        |
                      [100kΩ]
                        |
Audio In ──[10µF Cap]──-+──[10kΩ]──> MIC_PIN (GPIO 1)
                        |
                      [100kΩ]
                        |
                       GND
```

**How it works:**
1. The **coupling capacitor** (10µF) blocks any DC offset from the audio source
2. The **voltage divider** (2x 100kΩ) creates a DC bias at ~1.65V (midpoint of 3.3V range)
3. The audio signal now swings around 1.65V (e.g., 0.65V to 2.65V) - safely within the ADC range
4. The **protection resistor** (10kΩ, optional) limits current if the signal is too hot

**Important notes:**
- Use a **mono** signal or pick left/right channel. The ESP32 has one ADC input.
- For **stereo-to-mono** mixing, add a 10kΩ resistor on each channel before combining.
- Consumer line-level is ~1V peak. Professional (+4dBu) is ~1.7V peak - both work with this circuit.
- The firmware's **auto-gain** and **DC offset calibration** handle the rest automatically.

### Software Setup

```bash
# Switch to Line-In mode
audioinput linein

# Verify the setting
audioinput

# Set up audio-reactive patterns
S 15                    # Body: Audio VU Meter
mouth 14                # Mouth: Spectrum analyzer
audio 4                 # Full audio reactivity

# Save to flash
save
```

When you switch to Line-In mode, the firmware automatically:
- Adjusts the ADC mapping range for stronger line-level signals
- Lowers default sensitivity from 5 to 3
- Recalibrates DC offset on next boot

To switch back: `audioinput mic`

---

## Serial Commands Reference

Connect via Serial Monitor at **115200 baud**.

### System Commands

| Command | Description |
|---------|-------------|
| `help` | Show complete command reference |
| `status` | Display current settings |
| `save` | Save settings to flash |
| `restart` | Restart the system |
| `sysinfo` | Show system status (memory, health) |
| `eventlog` | Show event log |
| `eventlog clear` | Clear event log |

### Pattern Control

| Command | Description |
|---------|-------------|
| `S <0-19>` | Set body pattern |
| `next` | Next pattern |
| `prev` | Previous pattern |
| `nextanim` | Next animated pattern |
| `nextaudio` | Next audio-reactive pattern |
| `random` | Random pattern |

### Mouth Control

| Command | Description |
|---------|-------------|
| `mouth <0-14>` | Set mouth pattern |
| `mouthcolor <0-19>` | Set primary mouth color |
| `mouthcolor2 <0-19>` | Set secondary mouth color |
| `mouthsplit <0-4>` | Set split mode |
| `mouthbright <0-255>` | Set mouth brightness |
| `mouth on/off` | Enable/disable mouth |
| `talkspeed <1-10>` | Set talk animation speed |
| `smilewidth <1-10>` | Set smile width |

### Eye Control

| Command | Description |
|---------|-------------|
| `eyecolor <0-19>` | Set primary eye color |
| `eyecolor2 <0-19>` | Set secondary eye color |
| `eyemode <0-2>` | Set eye mode |
| `eyebrightness <50-200>` | Set eye brightness % |
| `eyeflicker on/off` | Toggle flicker effect |
| `eyeflickertime <min> <max>` | Set flicker timing (ms) |

### Audio Control

| Command | Description |
|---------|-------------|
| `audiomode <0-4>` | Set audio routing mode |
| `audioinput mic` | Switch to microphone input (default) |
| `audioinput linein` | Switch to line-in input (v5.1) |
| `audiosens <1-10>` | Set audio sensitivity |
| `audiothreshold <50-500>` | Set audio threshold manually |
| `autogain on/off` | Toggle auto-gain |

### Brightness Control

| Command | Description |
|---------|-------------|
| `B <0-255>` | Set global brightness |
| `bodybright <50-200>` | Set body brightness % |
| `mouthinner <50-200>` | Set mouth inner boost % |
| `mouthouter <50-200>` | Set mouth outer boost % |

### Demo Mode

| Command | Description |
|---------|-------------|
| `demo on` | Enable demo mode |
| `demo off` | Disable demo mode |
| `demotime <seconds>` | Set demo pattern duration |

#### Demo Mode Features

The advanced demo mode automatically cycles through body patterns, mouth patterns, and eye modes to showcase all capabilities.

- **Smart Pattern Cycling:** Curated selection of 13 body patterns and 10 mouth patterns
- **New v5.0 Patterns Included:** Plasma, Fire, Twinkle, Matrix, Heartbeat, Spectrum
- **Full Animation:** Cycles through different mouth animations synchronized with body patterns
- **Automatic Color Variation:** Colors change every few cycles for variety
- **Audio Patterns:** Demo includes Audio Sync and VU Meter patterns
- **State Preservation:** When demo mode is disabled, system continues with last active pattern

**Demo Pattern Rotation:**
```
Body: Random Blocks → Rainbow → Rainbow Glitter → Confetti → Juggle →
      Knight Rider → Breathing → Matrix Rain → Plasma → Fire →
      Twinkle → Audio Sync → Audio VU Meter

Mouth: Talk → Smile → Rainbow → Wave → Pulse → Sparkle →
       Matrix → Heartbeat → Audio Reactive → Spectrum
```

### Playlist Control

| Command | Description |
|---------|-------------|
| `playlist on/off` | Enable/disable playlist |
| `playlist show` | Show current playlist |
| `playlist <p,d;p,d;...>` | Set playlist (pattern,duration pairs) |
| `playlist save` | Save playlist to flash |
| `playlist load` | Load playlist from flash |

**Playlist Example:**
```
playlist 5,15;12,20;7,15;11,20
```
This creates a playlist: Rainbow (15s) → Breathing (20s) → Confetti (15s) → Knight Rider (20s)

### Preset Management (v3.1 Legacy)

| Command | Description |
|---------|-------------|
| `saveuser <1-3>` | Save to user preset |
| `loaduser <1-3>` | Load user preset |
| `deleteuser <1-3>` | Delete user preset |
| `listpresets` | List saved presets |

### v5.0 Extended Presets (10 Slots)

| Command | Description |
|---------|-------------|
| `preset save <1-10> [name]` | Save preset with optional name |
| `preset load <1-10>` | Load preset |
| `preset delete <1-10>` | Delete preset |
| `preset list` | Show all presets |

### Startup Sequence Control

| Command | Description |
|---------|-------------|
| `startup` | Show startup sequence status |
| `startup on` | Enable startup sequence |
| `startup off` | Disable startup sequence |

---

## Usage Examples

### Quick Start

Get up and running in seconds:

```bash
help                    # Show all available commands
status                  # Display current settings
demo on                 # Start the full demo mode
demotime 20             # Set demo to 20 seconds per pattern
```

### Basic Pattern Control

```bash
S 5                     # Set Rainbow pattern
next                    # Go to next pattern
random                  # Jump to random pattern
B 150                   # Set global brightness to 150
save                    # Save current settings
```

### Advanced Custom Setup

Create a fully customized look:

```bash
# Body configuration
S 16                    # Set to "Custom Block Sequence" pattern
bodybright 120          # Set body brightness to 120%

# Eye configuration
eyemode 2               # Set eyes to alternate between colors
eyecolor 3              # Set primary eye color to Warm White
eyecolor2 2             # Set secondary eye color to Blue
eyeflicker on           # Enable natural flicker effect
eyeflickertime 200 1200 # Set flicker timing

# Mouth configuration
mouth 7                 # Set mouth pattern to "Pulse"
mouthsplit 1            # Split mouth color vertically (left/right)
mouthcolor 2            # Set primary mouth color to Blue
mouthcolor2 0           # Set secondary mouth color to Red
mouthbright 100         # Set mouth brightness

# Save everything
save                    # Save the full configuration to flash
```

### Audio Reactive Setup

Configure for music/sound response:

```bash
# Body audio reactive
S 15                    # Set body to "Audio VU Meter"

# Mouth audio reactive
mouth 14                # Set mouth to "Spectrum" analyzer (v5.0)

# Audio system configuration
audio 4                 # Set everything to be audio reactive
sensitivity 6           # Set audio sensitivity to 6/10
autogain on             # Enable automatic gain control

# Alternative: Body audio sync with mouth spectrum
S 9                     # Body: "Audio Sync" (color changes)
mouth 14                # Mouth: "Spectrum" analyzer

save                    # Save settings
```

### Party/DJ Setup

High-energy configuration for events:

```bash
# Fast-changing patterns
S 14                    # Strobe pattern
mouth 11                # Sparkle mouth effect

# Or use audio reactive
S 9                     # Audio Sync body
mouth 3                 # Audio Reactive mouth
audio 4                 # Full audio reactivity
sensitivity 8           # High sensitivity for loud music

# Bright settings
B 255                   # Maximum brightness
mouthbright 200         # Boost mouth brightness
```

### Ambient/Display Setup

Calm, atmospheric configuration:

```bash
# Gentle patterns
S 12                    # Breathing pattern
mouth 7                 # Pulse mouth effect

# Warm colors
solidcolor 14           # Amber color for breathing
mouthcolor 3            # Warm White for mouth
eyecolor 14             # Amber eyes

# Subtle brightness
B 80                    # Lower overall brightness
eyeflicker off          # Disable eye flicker for calm look

save
```

### v5.0 New Patterns Showcase

Try the new patterns:

```bash
# Fire effect
S 18                    # Fire pattern (v5.0)
mouth 13                # Heartbeat mouth (v5.0)

# Plasma effect
S 17                    # Plasma pattern (v5.0)
mouth 12                # Matrix mouth (v5.0)

# Twinkle stars
S 19                    # Twinkle pattern (v5.0)
mouth 11                # Sparkle mouth

# Audio spectrum
S 15                    # Audio VU Meter body
mouth 14                # Spectrum analyzer mouth (v5.0)
audio 4                 # Full audio mode
```

### Playlist Configuration

Set up automated pattern sequences:

```bash
# Create a playlist: pattern,duration pairs separated by semicolons
playlist 5,15;17,20;18,15;12,20;19,15

# This creates:
# - Rainbow (5) for 15 seconds
# - Plasma (17) for 20 seconds
# - Fire (18) for 15 seconds
# - Breathing (12) for 20 seconds
# - Twinkle (19) for 15 seconds

playlist on             # Start the playlist
playlist show           # View current playlist
playlist save           # Save playlist to flash (persists after reboot)

# Later, load saved playlist
playlist load           # Restore saved playlist
playlist on             # Start it
```

### Preset Management

Save and recall complete configurations:

```bash
# Save current setup to preset slot 1 with a name
preset save 1 PartyMode

# Save another configuration
S 12
mouth 7
preset save 2 Ambient

# List all presets
preset list

# Load a preset
preset load 1           # Load "PartyMode"
preset load 2           # Load "Ambient"

# Delete a preset
preset delete 3
```

### System Monitoring

Check system health:

```bash
sysinfo                 # Show memory, loops/sec, health status
eventlog                # View system event history
eventlog clear          # Clear the event log
restart                 # Restart the system
```

---

## Color Palette (20 Colors)

| Index | Color | RGB Value |
|-------|-------|-----------|
| 0 | Red | 255, 0, 0 |
| 1 | Green | 0, 128, 0 |
| 2 | Blue | 0, 0, 255 |
| 3 | Warm White | 255, 230, 240 |
| 4 | Yellow | 255, 180, 0 |
| 5 | Cyan | 0, 255, 255 |
| 6 | Magenta | 255, 0, 255 |
| 7 | Orange | 255, 140, 0 |
| 8 | Purple | 128, 0, 128 |
| 9 | Pink | 255, 20, 147 |
| 10 | Black (Off) | 0, 0, 0 |
| 11 | Matrix Green | 0, 200, 100 |
| 12 | Ice Blue | 100, 150, 255 |
| 13 | UV Purple | 140, 0, 255 |
| 14 | Amber | 255, 120, 0 |
| 15 | Cool White | 255, 255, 255 |
| 16 | Lime Green | 120, 255, 0 |
| 17 | Teal | 0, 128, 128 |
| 18 | Pure White | 255, 255, 255 |
| 19 | Blue White | 180, 200, 255 |

---

## v5.0 New Features

### Universal Board Support ⭐

**Automatic board detection and configuration:**
- Single firmware works on both ESP32-C3 Mini and ESP32-S3 Mini
- Automatic pin configuration based on detected board
- FreeRTOS audio task automatically enabled on dual-core S3
- Board type displayed on startup

### FreeRTOS Thread Safety (ESP32-S3 Only)

**Dual-core optimization for ESP32-S3:**
- **LED Mutex:** Prevents race conditions during LED updates
- **Audio Task:** Dedicated task on Core 0 for consistent audio sampling at 200 Hz
- **Thread-Safe Updates:** All LED operations protected by semaphores
- **Performance:** Main loop on Core 1, audio on Core 0 for optimal performance

**Automatic behavior:**
- **ESP32-S3:** FreeRTOS enabled, uses dual cores
- **ESP32-C3:** FreeRTOS disabled (single-core, prevents LED flickering)

```cpp
#if ENABLE_FREERTOS_AUDIO  // Auto true on S3, false on C3
if (xSemaphoreTake(ledMutex, pdMS_TO_TICKS(LED_MUTEX_TIMEOUT_MS)) == pdTRUE) {
    FastLED.show();
    xSemaphoreGive(ledMutex);
}
#endif
```

### System Monitor

The system monitor tracks:

- **Free Heap Memory:** Current and minimum free RAM
- **Loop Performance:** Loops per second counter
- **Health Status:** Consecutive error tracking
- **Auto-Recovery:** Switches to safe mode on critical errors

**Memory Thresholds:**
- Warning: < 15,000 bytes
- Critical: < 10,000 bytes

**Auto-Recovery Behavior:**
1. After 5 consecutive errors → Switch to Safe Mode (LEDs Off)
2. After 10 consecutive errors → Automatic system restart

### Event Logger

Tracks system events with timestamps:

| Event Type | Description |
|------------|-------------|
| SYSTEM_START | System boot |
| PATTERN_CHANGE | Pattern switch |
| PRESET_LOAD | Preset loaded |
| PRESET_SAVE | Preset saved |
| ERROR | System error |
| MEMORY_WARNING | Low memory warning |

### Pattern Manager

Centralized pattern management with:

- **Category Classification:** Off, Static, Animated, Audio, Special
- **Category Navigation:** Jump between pattern types
- **Random Selection:** Random pattern with exclusion options

### Startup Sequence

Animated boot sequence with phases:

1. **Init** - Clear all LEDs
2. **LED Test** - RGB flash on all LEDs
3. **Eyes On** - Eyes fade in (orange)
4. **Mouth On** - Mouth lights from center outward
5. **Body Sweep** - Running light across panels
6. **Flash** - Final white flash
7. **Complete** - Ready for operation

---

## Configuration Constants

Located in `config.h`:

```cpp
// Board Detection (automatic)
#if defined(CONFIG_IDF_TARGET_ESP32C3)
    #define BOARD_TYPE "ESP32-C3 Mini"
    #define IS_DUAL_CORE false
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
    #define BOARD_TYPE "ESP32-S3 Mini"
    #define IS_DUAL_CORE true
#endif

// Hardware
#define NUM_LEDS_PER_PANEL 20
#define NUM_EYES 2
#define NUM_MOUTH_LEDS 80
#define TOTAL_BODY_LEDS 60

// FreeRTOS (auto-configured based on board)
#if IS_DUAL_CORE
    #define ENABLE_FREERTOS_AUDIO true  // S3 only
    #define AUDIO_TASK_CORE 0
#else
    #define ENABLE_FREERTOS_AUDIO false  // C3
#endif
#define AUDIO_TASK_STACK_SIZE 4096
#define AUDIO_TASK_PRIORITY 2
#define AUDIO_SAMPLE_INTERVAL_MS 5
#define LED_MUTEX_TIMEOUT_MS 100

// System Monitoring
#define ENABLE_MEMORY_MONITORING true
#define MEMORY_WARNING_THRESHOLD 15000
#define MEMORY_CRITICAL_THRESHOLD 10000
#define MAX_CONSECUTIVE_ERRORS 5

// Presets
#define MAX_PRESETS 10
#define PRESET_NAME_LENGTH 16

// Event Logger
#define MAX_LOG_ENTRIES 20

// Startup Sequence
#define STARTUP_SEQUENCE_ENABLED true
```

---

## File Structure

```
DJ-R3X/
├── DJ_Rex_ESP32_C3Mini_v3.1/          # Legacy ESP32-C3 specific firmware
│   └── (v3.1 files)
│
├── DJ_Rex_ESP32_S3Mini_v3.1/          # Legacy ESP32-S3 specific firmware
│   └── (v3.1 files)
│
├── DJ_Rex_ESP32_Unify_v5.0/           # ⭐ NEW: Universal firmware (C3/S3)
│   ├── DJ_Rex_ESP32_Unify_v5.0.ino    # Main program (auto-detects board)
│   ├── config.h                       # Hardware config + board detection
│   ├── globals.h / globals.cpp        # Global variables
│   ├── patterns_body.h / .cpp         # 20 body patterns
│   ├── patterns_mouth.h / .cpp        # 15 mouth patterns
│   ├── eyes.h / eyes.cpp              # Eye control
│   ├── audio.h / audio.cpp            # Audio processing
│   ├── helpers.h / helpers.cpp        # Utility functions
│   ├── serial_commands.h / .cpp       # Serial command handler
│   ├── settings.h / settings.cpp      # EEPROM persistence
│   ├── demo.h / demo.cpp              # Demo mode
│   │
│   │ # v5.0 New Modules
│   ├── event_logger.h / .cpp          # Event logging system
│   ├── preset_manager.h / .cpp        # 10-slot preset manager
│   ├── system_monitor.h / .cpp        # Health & memory monitoring
│   ├── pattern_manager.h / .cpp       # Pattern categorization
│   └── startup_sequence.h / .cpp      # Boot animation
│
└── README.md                          # This file
```

---

## Troubleshooting

### LEDs Not Working

1. **Check Wiring:**
   - Data line connected to correct GPIO
   - 5V and GND properly connected
   - Resistor on data line (300-500Ω)

2. **Check Power:**
   - Adequate power supply (5V, 5A+)
   - Power injection for long strips
   - Capacitor near power input

3. **Check Software:**
   - FastLED version 3.9.0+
   - Correct LED count in config.h
   - Correct color order (GRB for WS2812B)

### Audio Not Responding

1. **Check Microphone:**
   - Connected to Pin 1 (MIC_PIN)
   - Powered correctly
   - Not muted/blocked

2. **Check Settings:**
   - Audio mode not set to OFF
   - Threshold not too high
   - Sensitivity appropriate

3. **Serial Debug:**
   - Use `status` command
   - Check audio level readings
   - Verify threshold values

### System Crashes/Restarts

1. **Memory Issues:**
   - Check `sysinfo` for free heap
   - Reduce pattern complexity
   - Check for memory leaks

2. **Power Issues:**
   - Ensure adequate current supply
   - Add power filtering capacitors
   - Check for voltage drops

### Patterns Not Saving

1. **Flash Issues:**
   - Use `save` command after changes
   - Check EEPROM write errors in log
   - Clear and reinitialize preferences

---

## Version History

### v5.1.0 "Line-In Edition" (2026-02-21)

**New Feature: Line-In Audio Input**

- **Line-In support:** Connect directly to a mixer, DJ controller, or media player via a simple adapter circuit
- **`audioinput` command:** Switch between Microphone and Line-In modes via serial menu
- **Automatic sensitivity adjustment:** Firmware adapts mapping range and sensitivity defaults per input mode
- **Persistent setting:** Audio input mode is saved to flash and survives reboots
- **Full documentation:** Audio-to-Light signal flow, Line-In adapter schematic, and tuning guide added to README

### v5.0.0 "Enhanced Edition" (2025-11-30)

**Major Feature Release**

#### Universal Board Support ⭐
- **Single firmware for ESP32-C3 Mini AND ESP32-S3 Mini**
- Automatic board detection and pin configuration
- FreeRTOS automatically enabled on S3 (dual-core)
- Board type displayed on startup serial output

#### New Patterns
- **Body:** Plasma, Fire, Twinkle (patterns 17-19)
- **Mouth:** Matrix, Heartbeat, Spectrum (patterns 12-14)

#### New Modules
- **Event Logger:** System event tracking
- **Preset Manager:** 10 preset slots (up from 3)
- **System Monitor:** Memory and health monitoring
- **Pattern Manager:** Categorized pattern navigation
- **Startup Sequence:** Animated boot animation

#### FreeRTOS Integration (ESP32-S3)
- Dedicated audio task on Core 0 (S3 only)
- LED mutex for thread safety (S3 only)
- Improved timing consistency
- Disabled on C3 to prevent LED flickering

#### Auto-Recovery
- Safe mode on critical errors
- Automatic restart on system failure
- Memory warning system

#### Extended Commands
- `nextanim`, `nextaudio`, `random` - Category navigation
- `preset save/load/delete/list` - 10 preset slots
- `playlist save/load` - Persistent playlists
- `sysinfo`, `eventlog` - System monitoring
- `startup on/off` - Boot sequence control
- `restart` - Manual system restart

#### Demo Mode Improvements
- Extended pattern rotation (13 body, 10 mouth)
- Includes all new v5.0 patterns
- Automatic color variation

---

### v3.1.0 "Playlist Edition" (Previous)

- Playlist system with smooth transitions
- User presets (3 slots)
- Extended mouth patterns (12 total)
- Dual-color eye modes
- Eye flicker effect

---

## Credits

- **Hardware Design:** Printed-Droid.com
- **Firmware:** Printed-Droid Development Team
- **FastLED Library:** Daniel Garcia and Mark Kriegsman
- **ESP32 Platform:** Espressif Systems

---

## License

This firmware is provided for personal, non-commercial use with Printed-Droid hardware.

---

## Support

- **Website:** [www.printed-droid.com](https://www.printed-droid.com)
- **Community:** Printed Droid on Facebook
- **Issues:** Report bugs via GitHub Issues

---

**May the Force be with you!** 🌟
