---
title: 'Window height resize for Status logs'
type: 'feature'
created: '2026-07-18'
status: 'done'
route: 'one-shot'
context: []
---

# Window height resize for Status logs

## Intent

**Problem:** The main window was locked at 500×500, so the Status log could not grow when more MIDI activity needed to be visible.

**Approach:** Allow vertical resize only (fixed 500 px width, minimum height 500), with leftover height going to the existing Status log layout; drop the maximise button so fullscreen cannot break the width lock.

## Suggested Review Order

- Entry: height-only limits and no maximise button
  [`Main.cpp:745`](../../Source/Main.cpp#L745)

- Corner grip + resize limits bind width and min/max height
  [`Main.cpp:757`](../../Source/Main.cpp#L757)

- Existing top-fixed layout; Status editor already takes leftover height
  [`Main.cpp:363`](../../Source/Main.cpp#L363)
