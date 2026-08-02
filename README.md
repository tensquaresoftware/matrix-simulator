# Matrix-Simulator

Standalone macOS tool that answers Universal Device Inquiry as a Matrix-1000,
Matrix-6/6R (provisional), or Unknown Device over MIDI (IAC). Useful for
development and travel without Matrix hardware.

## Build

```bash
cmake --preset macos-debug-arm64
cmake --build --preset macos-debug-arm64
```

App:

`Builds/macOS/ARM/Debug/Matrix-Simulator_artefacts/Debug/Matrix-Simulator.app`

## Usage

1. Enable **IAC Driver** in Audio MIDI Setup.
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

Prefer distinct IAC buses for From vs To when possible.

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
- Windows / Linux virtual MIDI
- Shipping as a plugin or inside another product’s AU / VST3 / Standalone binary
