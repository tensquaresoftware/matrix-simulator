---
title: 'Show app version label on button row'
type: 'feature'
created: '2026-09-19'
status: 'done'
route: 'one-shot'
---

# Show app version label on button row

## Intent

**Problem:** The UI did not show the shipped app version, so users could not tell which build they were running while configuring MIDI ports.

**Approach:** Bump the CMake project version to 1.1.0 (SSOT), expose it via JUCE application version macros, and show a right-aligned `(App v…)` label on the same row as Refresh Ports and Clear Logs.

## Suggested Review Order

**Version SSOT**

- Project version bumped to 1.1.0 as the single source of truth
  [`CMakeLists.txt:18`](../../CMakeLists.txt#L18)

- Compile defs wire product name/version into the app target
  [`CMakeLists.txt:68`](../../CMakeLists.txt#L68)

- Application APIs read the same macros (no hardcoded version string)
  [`Main.cpp:786`](../../Source/Main.cpp#L786)

**UI label**

- Label text uses compile-time concat of the version macro
  [`Main.cpp:274`](../../Source/Main.cpp#L274)

- Same row as port buttons, with gap and remaining width right-aligned
  [`Main.cpp:424`](../../Source/Main.cpp#L424)
