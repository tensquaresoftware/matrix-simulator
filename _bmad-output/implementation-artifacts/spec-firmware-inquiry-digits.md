---
title: 'Firmware inquiry digits (no decimal)'
type: 'bugfix'
created: '2026-08-02'
status: 'done'
route: 'one-shot'
context: []
---

# Firmware inquiry digits (no decimal)

## Intent

**Problem:** The simulator copied the Firmware field as left-aligned ASCII (including a decimal point), so Matrix-Control showed `V1.20` while real Oberheim hardware sends right-justified digits without a point (`V120`).

**Approach:** Restrict the Firmware field to at most four digits, pack Device Inquiry `<rev0..3>` as right-justified ASCII with leading spaces, and document the wire format.

## Suggested Review Order

**Wire packing**

- Digits extract / normalize / right-justify pack SSOT
  [`DeviceInquiry.h:47`](../../Source/DeviceInquiry.h#L47)

- `encodeReply` uses pack helper; default `"111"`
  [`DeviceInquiry.h:85`](../../Source/DeviceInquiry.h#L85)

**UI and prefs**

- Digits-only input + default `111`
  [`Main.cpp:245`](../../Source/Main.cpp#L245)

- Legacy dotted prefs migrate on load and persist
  [`Main.cpp:335`](../../Source/Main.cpp#L335)

- Empty mid-edit keeps last committed value
  [`Main.cpp:456`](../../Source/Main.cpp#L456)

**Verification aid**

- Status log includes packed rev hex bytes
  [`Main.cpp:695`](../../Source/Main.cpp#L695)

**Docs**

- Protocol note: unpadded UI digits vs padded wire; breaking vs old sim
  [`README.md:43`](../../README.md#L43)
