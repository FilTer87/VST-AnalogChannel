# AnalogChannel

AnalogChannel is a VST3 channel strip plugin designed for mixing engineers who demand **fast and characterful results** in their workflow.

Each section provides carefully limited controls: fixed attack/release times, discrete frequency selections, and pre-configured parameters (both fixed and dynamic) working under the hood to deliver professional results with most source material.
This is your **go-to channel strip for fast, colored, musical mixing decisions**.

It is **not** intended for surgical precision!

Built with faithful ports of legendary algorithms from **Airwindows** and classic **JSFX processors**. See `CREDITS.md` for details

### Key Features

- **Dual-Mono Architecture**: Left and right channels process independently, enabling analog channel modeling through the Channel Variation system
- **Channel Variation System**: Emulates natural component tolerances in analog consoles with 48 unique channel presets (see `UserManual.md` for details)
- **Independent Bypass**: All sections feature smooth 10ms crossfade CPU-efficient bypass, for A/B comparison without clicks and resource optimization.
- **Flexible Signal Routing**: Style Comp. can be placed pre or post-EQ, filters can be routed post-OutStage for creative effects
- **Comprehensive Metering**: Input/output peak meters plus dedicated GR meters for dynamics sections
- **Full DAW Automation**: All parameters support VST3 automation and MIDI control
- **Zero Latency**: Real-time processing suitable for tracking and mixing

### Processing sections:

- **Input stage saturation**: 4 modes to choose between Clean (digital gain), Pure (transparent saturation), Tape (vintage reel saturation with flutter and head bump), and Tube (3-stages asymmetric harmonics); DSP ported from AirWindows
- **HPF/LPF filters** with "Bump" option (higher Q factor) and switchable slopes
- **"Clean" Compressor**: Transparent peak control for taming dynamics before EQ
- **Low Dynamic custom design section**: Dual-mode dynamics processor for signals *below the threshold*; a custom 1-knob approach that works as either a downward expander/gate or an upward compressor
- **4 bands EQ with custom design**: Musical baxandall shelving + parametric EQ, using custom curves with dynamic Q (similar to an API-style EQ); Mid-Cut control for precise mid-range shaping added to the original baxandall shelves.
- **Style Compressor** with Two character compression modes: "Warm" (optical CL1B-style) that with the coloration and adaptive attack/release time adds warmth, glue and vibes, and "Punch" mode to enhance the initial parts of the transients on percussive elements
- **Console Emulation**: Choose one of the magic console tones between Essex (Neve), Oxford (SSL style), and USA (API); DSP ported from AirWindows
- **Output Stage** - Final stage with saturation and clipping options (Hard/Soft); DSP ported from AirWindows

For detailed features, usage examples and in depth technical information, read the `UserManual.md`

If you like AnalogChannel, please consider supporting the project: https://buymeacoffee.com/oz3watvqah

---

**License**: GPL v3 | **Developer**: Filippo Terenzi - KuramaSound | **Formats**: VST3 (Windows/macOS/Linux)
## Linux build and install

AnalogChannel now ships with a CMake-based build that works on Linux (VST3 + optional standalone test host).

### Prerequisites
- JUCE 7.x available either via `juce-config` package or a local JUCE clone (set `JUCE_DIR=/path/to/JUCE`)
- CMake ≥ 3.15
- Toolchain: `build-essential` (or clang), `pkg-config`, `alsa`/`pipewire-jack` dev packages, X11 dev headers

### Quick build
```bash
# from the repo root
./scripts/build-linux.sh
```
This will configure + build in `./build/` and install to `~/.local` (`~/.local/lib/vst3/AnalogChannel.vst3`). Override with `PREFIX=/custom/prefix ./scripts/build-linux.sh`.

Manual CMake invocation:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DJUCE_DIR=/path/to/JUCE -DANALOGCHANNEL_STANDALONE=ON
cmake --build build --config Release --parallel
cmake --install build --config Release --prefix ~/.local
```

### LV2 status
JUCE does not provide an official LV2 target. Bringing LV2 support would require an external wrapper (e.g. DPF/distribution or a JUCE-LV2 fork). No LV2 binary is produced in this repo, but the CMake layout keeps the code ready should such a wrapper be added later.

See `CREDITS.md` for detailed third-party algorithm attributions.
