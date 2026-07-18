# Matrix-Simulator

Standalone macOS tool that answers Universal Device Inquiry as a Matrix-1000 or
Matrix-6/6R (provisional) over MIDI (IAC). Useful for development and travel without
Matrix hardware.

## Build

```bash
cmake --preset macos-debug-arm64
cmake --build --preset macos-debug-arm64
```

App:

`Builds/macOS/ARM/Debug/Matrix-Simulator_artefacts/Debug/Matrix-Simulator.app`

## Usage

1. Enable **IAC Driver** in Audio MIDI Setup.
2. Launch **Matrix-Simulator**. Choose **Synth Profile** (Matrix-1000 or Matrix-6/6R provisional).
3. Select **MIDI From** then **MIDI To**.
4. Optional: **Options → Active MIDI Ports…** — checkboxes filter which ports appear in the combos.
5. Point a host editor’s MIDI output at the simulator’s **MIDI To**, and its MIDI input at the simulator’s **MIDI From**.
6. Send Universal Device Inquiry; expect a Device ID reply matching the selected profile.

```
Host editor  --MIDI To-->  Simulator "MIDI To"
Host editor  <--MIDI From--  Simulator "MIDI From"
```

Prefer distinct IAC buses for To vs From when possible.

## Protocol

- Request: `F0 7E 7F 06 01 F7`
- Reply: `F0 7E <chan> 06 02 10 06 00 <memb-lo> <memb-hi> <rev0..3> F7`
- Member / family bytes are defined only in `Source/DeviceInquiry.h`.
  Update them here if the Matrix Device Inquiry protocol bytes change.

## Out of scope (for now)

- Full patch / master dump emulation
- Remote Parameter Edit / Program Change beyond ignore-or-log
- Distinct Matrix-6R inquiry pattern
- Windows / Linux virtual MIDI
- Shipping as a plugin or inside another product’s AU / VST3 / Standalone binary
