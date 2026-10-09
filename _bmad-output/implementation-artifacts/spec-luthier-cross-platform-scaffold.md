---
title: 'Luthier-style cross-platform scaffold (GUI app)'
type: 'chore'
created: '2026-10-09'
status: 'done'
baseline_commit: '11f04ec8bccd32cf3a800cfe202df0ff872c5764'
review_loop_iteration: 0
context:
  - '{project-root}/CMakeLists.txt'
  - '{project-root}/CMakeUserPresets.json'
  - '{project-root}/.cursor/rules/matrix-simulator-project.mdc'
---

<frozen-after-approval reason="human-owned intent — do not modify unless human renegotiates">

## Intent

**Problem:** Matrix-Simulator only has macOS Apple Silicon CMake presets and a thin CMakeLists, so Windows and Linux cannot be built the Luthier way. It was never Luthier-scaffolded (no `.luthier.json`).

**Approach:** Keep `juce_add_gui_app` and existing `Source/`. Graft Luthier multi-platform CMake, artefacts copy, sidecar, IDE helpers, and docs — adapted for a GUI app, not a plugin conversion.

## Boundaries & Constraints

**Always:**
- Stay a GUI app (`juce_add_gui_app`) with current sources.
- JUCE: macOS `…/JUCE-9`; Windows/Linux `…/JUCE` (keep env/`JUCE_DIR` priority + standard fallbacks).
- Artefacts copy ON by default to Program-Changer Dropbox paths; dest `{ARTEFACTS_DEST}/Standalone/Matrix-Simulator.{app,exe,}`.
- Gui_app **build** tree has no `/Standalone/` under `*_artefacts/`; only the Dropbox **dest** uses `Standalone/`.
- Keep C++20 and version `1.2.0`. English in repo files.

**Ask First:** Changing JUCE path strategy, defaulting artefacts copy OFF, or converting to `juce_add_plugin` / AU/VST3.

**Never:** Plugin/Processor–Editor rewrite; VST3 elevated/system copy; MIDI/UI logic changes; sibling-repo dependencies.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|--------------|---------------------------|----------------|
| macOS configure | `cmake --preset macos-debug-arm64`, JUCE-9 present | Config under `Builds/macOS/ARM/Debug` | FATAL if JUCE missing |
| Windows / Linux configure | Matching host + `windows-debug` / `linux-debug` | `Builds/Windows` or `Builds/Linux/Debug` | Preset condition skips off-host |
| Artefacts copy | Copy ON, successful build | File under `{ARTEFACTS_DEST}/Standalone/` | CMake copy failure fails that custom target |
| Wrong macOS arch preset | Luthier host/binaryDir mismatch | Configure FATAL with preset hint | Same as Program-Changer guards |

</frozen-after-approval>

## Code Map

- `CMakeLists.txt` — generators, artefacts copy for gui_app; keep `juce_add_gui_app`
- `CMakeUserPresets.json` — full Luthier preset set + `ARTEFACTS_DIR_*`
- `.luthier.json` — write-only sidecar (gui_app; plugin flags empty/false)
- `.vscode/tasks.json`, `launch.json`, `settings.json` — cmake tasks; App launch Win/Linux; no DAW configs
- `.gitignore` — merge Luthier Win/CMake ignores; keep Matrix Cursor/`_local` rules
- `README.md` — multi-OS build + artefacts; still a GUI app
- `.cursorrules`, `.cursor/rules/matrix-simulator-project.mdc` — multi-OS wording
- Read-only refs: `…/Program-Changer/`, `…/Luthier/templates/`

## Tasks & Acceptance

**Execution:**
- [x] `CMakeLists.txt` -- WIN32 generator, macOS arch guards, `ARTEFACTS_DIR_*` + `CopyToArtefactsDir` (gui_app source paths); no system VST3 copy
- [x] `CMakeUserPresets.json` -- Full preset set (macOS arm64/x86_64/universal × D/R, Windows×4, Linux×2) with Dropbox `ARTEFACTS_DIR_*`
- [x] `.luthier.json` -- Sidecar: identity, JUCE/artefacts paths, C++20, no plugin formats
- [x] `.vscode/*` -- Luthier cmake tasks; Build: App → `Matrix-Simulator`; Win/Linux launch paths
- [x] `.gitignore` -- Merge Luthier entries; keep Matrix-specific ignores
- [x] `README.md` -- Per-OS presets, binary paths, artefacts; not a plugin
- [x] `.cursorrules` + `matrix-simulator-project.mdc` -- Multi-OS Luthier-style GUI scaffold

**Acceptance Criteria:**
- Given clean macOS tree, when `macos-debug-arm64` configure+build, then app still builds.
- Given presets file, when listed on each OS, then that OS’s presets appear; file contains windows-* and linux-*.
- Given artefacts ON after build, when checking Dropbox path for OS/arch, then `Standalone/Matrix-Simulator` artefact exists.
- Given `.luthier.json`, when read, then JUCE-9 macOS and Win/Linux `…/JUCE` match Always.
- Given Source/, when compared to before, then no MIDI/UI rewrite and no `juce_add_plugin`.

## Spec Change Log

## Design Notes

Gui_app artefact **sources** (dest still under `Standalone/`):

```
…/Matrix-Simulator_artefacts/$<CONFIG>/Matrix-Simulator.app   # macOS
…/$<CONFIG>/Matrix-Simulator.exe                               # Windows
…/$<CONFIG>/Matrix-Simulator                                   # Linux
```

No Rosetta presets (absent in Program-Changer); keep Intel-Rosetta **guards** if binaryDir patterns match.

## Verification

**Commands:**
- `cmake --preset macos-debug-arm64` -- configure OK
- `cmake --build --preset macos-debug-arm64` -- app builds
- `cmake --list-presets` -- macOS presets on Darwin; JSON has windows-* / linux-*
- Grep `CMakeLists.txt` — no `juce_add_plugin`, no VST3 elevated copy

**Manual checks (if no CLI):**
- On Windows/Linux later: matching preset configure+build; confirm Dropbox `Standalone/` copy

## Suggested Review Order

**App identity (still GUI app)**

- Version SSOT bumped to 1.2.0 for this scaffold release.
  [`CMakeLists.txt:48`](../../CMakeLists.txt#L48)

- Product target remains `juce_add_gui_app`, not a plugin.
  [`CMakeLists.txt:111`](../../CMakeLists.txt#L111)

**Multi-OS CMake**

- Windows default generator (VS 2026) mirrors Luthier.
  [`CMakeLists.txt:41`](../../CMakeLists.txt#L41)

- macOS JUCE-9 vs Win/Linux JUCE paths preserved.
  [`CMakeLists.txt:76`](../../CMakeLists.txt#L76)

- Artefacts copy runs POST_BUILD so Build: App / F5 also copy.
  [`CMakeLists.txt:193`](../../CMakeLists.txt#L193)

**Presets & sidecar**

- Full macOS / Windows / Linux configure preset set.
  [`CMakeUserPresets.json:5`](../../CMakeUserPresets.json#L5)

- Sidecar snapshot with JUCE-9 macOS and artefacts ON.
  [`.luthier.json:4`](../../.luthier.json#L4)

**IDE & docs**

- CMake Tools tasks retargeted to `Matrix-Simulator`.
  [`.vscode/tasks.json:5`](../../.vscode/tasks.json#L5)

- Multi-OS build + artefacts documented; still a GUI app.
  [`README.md:1`](../../README.md#L1)
