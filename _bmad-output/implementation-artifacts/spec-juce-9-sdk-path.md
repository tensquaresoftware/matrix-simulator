---
title: 'Switch default macOS JUCE SDK to JUCE-9'
type: 'chore'
created: '2026-09-19'
status: 'done'
route: 'one-shot'
---

# Switch default macOS JUCE SDK to JUCE-9

## Intent

**Problem:** Configure failed because the default macOS JUCE path pointed at a removed `…/SDKs/JUCE/` tree while the machine now has versioned folders `JUCE-8` and `JUCE-9`.

**Approach:** Point `JUCE_DIR_MACOS` at `/Volumes/Guillaume/Dev/SDKs/JUCE-9` and update project agent notes from JUCE 8 to JUCE 9. Wipe the stale Debug build tree once so CMake is not pinned to the old JUCE-8 source path.

## Suggested Review Order

**Build path**

- Default macOS SDK path `JUCE-9`
  [`CMakeLists.txt:23`](../../CMakeLists.txt#L23)

**Agent / project notes**

- JUCE 9 API reference
  [`.cursorrules:26`](../../.cursorrules#L26)

- Environment blurb with `JUCE-9` example path
  [`.cursor/rules/matrix-simulator-project.mdc:10`](../../.cursor/rules/matrix-simulator-project.mdc#L10)

- Standards description + verify-against JUCE 9
  [`.cursor/rules/cpp-juce-standards.mdc:2`](../../.cursor/rules/cpp-juce-standards.mdc#L2)
