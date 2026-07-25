---
title: 'Unknown Device synth profile'
type: 'feature'
created: '2026-07-25'
status: 'done'
route: 'one-shot'
context: []
---

# Unknown Device synth profile

## Intent

**Problem:** Synth Profile only offered Matrix-1000 and Matrix-6/6R, so there was no way to exercise Matrix-Control’s path for a device that answers Universal Device Inquiry but is neither known Matrix model.

**Approach:** Add an **Unknown Device** profile that replies with the same Oberheim manufacturer/family as the Matrix profiles, but member bytes `00 00`, and document those bytes in the README Protocol section.

## Suggested Review Order

- Member SSOT for Unknown Device (`00 00`)
  [`DeviceInquiry.h:29`](../../Source/DeviceInquiry.h#L29)

- Profile enum + combo ID mapping (item 3)
  [`Main.cpp:29`](../../Source/Main.cpp#L29)

- UI item and prefs load/change use the shared mapper
  [`Main.cpp:247`](../../Source/Main.cpp#L247)

- Inquiry reply uses profile member helpers; Status log shows hex members
  [`Main.cpp:682`](../../Source/Main.cpp#L682)

- Protocol docs list per-profile member bytes
  [`README.md:39`](../../README.md#L39)
