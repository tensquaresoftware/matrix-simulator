# Matrix-Simulator

Version: 1.2.0  
Manufacturer: Ten Square Software  
Type: Standalone GUI app (`juce_add_gui_app`) — not an AU/VST3 plugin

Answers Universal Device Inquiry as a Matrix-1000, Matrix-6/6R (provisional),
or Unknown Device over MIDI (IAC / OS MIDI). Useful for development and travel
without Matrix hardware.

Scaffold follows Luthier-style multi-platform CMake (presets, artefacts copy,
`.luthier.json` sidecar). Product code remains a GUI app.

## Build Instructions

### Prerequisites

#### Windows

- Windows 10 or later
- **Cursor** or **VS Code** with **CMake Tools** (and C/C++ debugging if you use **F5**)
- **CMake 4.2+** (required for the default `windows-debug` / `windows-release` presets targeting **Visual Studio 2026**)
- **Visual Studio 2026** with **Desktop development with C++** workload (default presets)
- **Legacy:** if you only have **Visual Studio 2022**, use CMake 3.22+ and select `windows-debug-vs2022` / `windows-release-vs2022`
- JUCE at `C:/Users/Guillaume/Dev/SDKs/JUCE` (or set `JUCE_DIR`)

#### macOS

- macOS 13 or later
- **Cursor** or **VS Code** with **CMake Tools**
- CMake 3.22+, Ninja
- JUCE 9 at `/Volumes/Guillaume/Dev/SDKs/JUCE-9` (or set `JUCE_DIR`)

#### Linux

- Linux (e.g. Ubuntu 22.04)
- **Cursor** or **VS Code** with **CMake Tools**
- CMake 3.22+, Ninja, GDB-capable toolchain (example: `sudo apt install ninja-build build-essential gdb`)
- JUCE at `/home/guillaume/Dev/SDKs/JUCE` (or set `JUCE_DIR`)

### Environment Setup

JUCE is resolved in order: CMake cache / `-DJUCE_DIR`, env `JUCE_DIR`, per-OS
paths in `CMakeLists.txt`, then platform defaults (`/Applications/JUCE`,
`C:/Program Files/JUCE`, `/usr/local/JUCE`).

### Build

Build directories are separated by platform and architecture
(`Builds/Windows`, `Builds/macOS/ARM`, `Builds/macOS/Intel`,
`Builds/macOS/Universal`, `Builds/Linux`) to avoid mixing outputs.

#### macOS Apple Silicon

```bash
cmake --preset macos-debug-arm64
cmake --build --preset macos-debug-arm64
```

App:

`Builds/macOS/ARM/Debug/Matrix-Simulator_artefacts/Debug/Matrix-Simulator.app`

#### macOS Intel

```bash
cmake --preset macos-debug-x86_64
cmake --build --preset macos-debug-x86_64
```

#### macOS Universal

```bash
cmake --preset macos-debug-universal
cmake --build --preset macos-debug-universal
```

#### Windows

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug --config Debug
```

App:

`Builds/Windows/Matrix-Simulator_artefacts/Debug/Matrix-Simulator.exe`

#### Linux

```bash
cmake --preset linux-debug
cmake --build --preset linux-debug
```

App:

`Builds/Linux/Debug/Matrix-Simulator_artefacts/Debug/Matrix-Simulator`

### Artefacts copy

When `COPY_TO_ARTEFACTS_DIR` is ON (default), the built app is copied after
build to the central Dropbox artefacts folder under `Standalone/`, for example:

- macOS ARM: `…/Artefacts/macOS/ARM/Standalone/Matrix-Simulator.app`
- Windows: `…/Artefacts/Windows/Standalone/Matrix-Simulator.exe`
- Linux: `…/Artefacts/Linux/Standalone/Matrix-Simulator`

Paths are set in `CMakeLists.txt` / `CMakeUserPresets.json` (`ARTEFACTS_DIR_*`).

## Usage

1. Enable a virtual MIDI path (macOS: **IAC Driver** in Audio MIDI Setup).
2. Launch **Matrix-Simulator**. Choose **Synth Profile** (Matrix-1000, Matrix-6/6R provisional, or Unknown Device).
3. Select **MIDI From** then **MIDI To**.
4. Optional: **Options → Active MIDI Ports…** — checkboxes filter which ports appear in the combos.
5. Point a host editor’s MIDI output at the simulator’s **MIDI From**, and its MIDI input at the simulator’s **MIDI To**.
   (With Matrix-Control, whose From/To labels stay synth-centric, that means crossing the port picks between the two apps.)
6. Send Universal Device Inquiry; expect a Device ID reply matching the selected profile.

```
Host editor Out  -->  Simulator "MIDI From"
Host editor In   <--  Simulator "MIDI To"
```

Prefer distinct virtual buses for From vs To when possible.

## Protocol

- Request: `F0 7E 7F 06 01 F7`
- Reply: `F0 7E <chan> 06 02 10 06 00 <memb-lo> <memb-hi> <rev0..3> F7`
- Manufacturer `10`, family `06 00` for all profiles. Member bytes by **Synth Profile**:
  - Matrix-1000 → `02 00`
  - Matrix-6/6R (provisional) → `01 00`
  - Unknown Device → `00 00` (Oberheim-family reply that is neither Matrix model)
- `<rev0..3>` = up to 4 ASCII digits, **right-justified** with leading spaces (no decimal
  point). UI stores unpadded digits (default `111` = human 1.11); wire example
  `111` → `20 31 31 31` (` 111`). Older simulator builds that sent a dotted string
  (e.g. `1.11`) are intentionally incompatible with this hardware-accurate packing.
- Member / family bytes are defined only in `Source/DeviceInquiry.h`.
  Update them here if the Matrix Device Inquiry protocol bytes change.

## Out of scope (for now)

- Full patch / master dump emulation
- Remote Parameter Edit / Program Change beyond ignore-or-log
- Distinct Matrix-6R inquiry pattern
- Virtual MIDI setup beyond standard OS ports (IAC, loopMIDI, ALSA/JACK buses, etc.)
- Shipping as a plugin or inside another product’s AU / VST3 / Standalone binary
