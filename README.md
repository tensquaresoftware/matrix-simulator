# Matrix-Simulator

Standalone macOS tool that answers Universal Device Inquiry as a Matrix-1000 or Matrix-6
so [Matrix-Control](https://github.com/tensquaresoftware/Matrix-Control) can unlock and show
the correct device identity without hardware.

Formerly lived under `Matrix-Control/Tools/`; now a separate project that can grow beyond
inquiry-only responses.

## Build

```bash
cmake --preset macos-debug-arm64
cmake --build --preset macos-debug-arm64
```

App:

`Builds/macOS/ARM/Debug/Matrix-Simulator_artefacts/Debug/Matrix-Simulator.app`

## Usage with Matrix-Control

1. Enable **IAC Driver** in Audio MIDI Setup.
2. Launch **Matrix-Simulator**. Choose **Synth Profile** (Matrix-1000 or Matrix-6 provisional).
3. Select **MIDI From** then **MIDI To**.
4. Optional: **Options → Active MIDI Ports…** — checkboxes filter which ports appear in the combos.
5. In Matrix-Control Standalone, set the same To / From ports.
6. Expect unlock + footer identity; Matrix-6 grays Bank Utility.

```
Matrix-Control  --MIDI To-->  Simulator "MIDI To"
Matrix-Control  <--MIDI From--  Simulator "MIDI From"
```

Prefer distinct IAC buses for To vs From when possible.

## Protocol

- Request: `F0 7E 7F 06 01 F7`
- Reply: `F0 7E <chan> 06 02 10 06 00 <memb-lo> <memb-hi> <rev0..3> F7`
- Member / family bytes are local to this tool (`Source/DeviceInquiry.h`).
  Update them manually if the Matrix Device Inquiry protocol bytes change.

## Out of scope (for now)

- Full patch / master dump emulation
- Remote Parameter Edit / Program Change beyond ignore-or-log
- Distinct Matrix-6R inquiry pattern
- Windows / Linux virtual MIDI
- Shipping inside Matrix-Control AU / VST3 / product Standalone
