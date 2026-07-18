---
title: 'MIDI From/To local app referential'
type: 'feature'
created: '2026-07-18'
status: 'done'
baseline_commit: '0f2babc058941ca09f56c141d44ba32729ef276c'
review_loop_iteration: 0
context: []
---

<frozen-after-approval reason="human-owned intent — do not modify unless human renegotiates">

## Intent

**Problem:** Matrix-Simulator labels **MIDI From** / **MIDI To** from the Matrix synth’s point of view (From = app output, To = app input). That matches Matrix-Control’s editor wording, so both apps need identical port picks — which feels like a wiring bug.

**Approach:** Redefine the two combos in the simulator’s own I/O frame: **MIDI From** = listen (MidiInput), **MIDI To** = send replies (MidiOutput). Update logs, README, and one-shot preference migration so existing saved port IDs keep working with the same host-editor settings after upgrade.

## Boundaries & Constraints

**Always:**
- **MIDI From** opens a `MidiInput` and populates from available inputs (subject to Active MIDI inputs filter).
- **MIDI To** opens a `MidiOutput` and populates from available outputs (subject to Active MIDI outputs filter).
- Log / error strings must match the new meaning (listen on From, reply on To).
- On first load after this change, migrate saved `midiFromIdentifier` / `midiToIdentifier` by swapping them once, then persist a flag so migration never runs again.
- README usage diagram must describe host editor Out → simulator From, host editor In ← simulator To (crossed relative to Matrix-Control’s synth-centric labels).
- UI label text strings stay `"MIDI From"` / `"MIDI To"`; only semantics and wiring change.
- Main Options layout keeps **MIDI From** above **MIDI To** (From first, To directly below) — do not reverse visual order when rewiring semantics.
- English-only source and docs.

**Ask First:**
- Changing Matrix-Control (sibling product) — out of this repo; do not invent Control-side edits.
- Renaming prefs keys instead of swap+flag migration.

**Never:**
- Change Device Inquiry protocol bytes or profile/firmware behavior.
- Relabel Active MIDI Ports dialog (inputs/outputs stay OS-direction names).
- French strings in code or README.
- Broader MIDI stack refactor beyond From/To wiring, logs, prefs migration, and README.

## I/O & Edge-Case Matrix

| Scenario | Input / State | Expected Output / Behavior | Error Handling |
|----------|--------------|---------------------------|----------------|
| Fresh install | No saved MIDI IDs, no migration flag | Combos default to (none); From lists inputs, To lists outputs | N/A |
| Upgrade with old prefs | From ID was output port A, To ID was input port B; flag absent | Swap → From=B (input), To=A (output); set migration flag; reopen ports | If an ID no longer exists, combo shows (none) for that side |
| Already migrated | Flag present | Load From/To IDs as-is (no swap) | N/A |
| Inquiry, To closed | Valid From open, To (none) | Inquiry received; no SysEx reply; log that MIDI To is not open | No crash |
| Same virtual name both sides | User picks same display name for From and To | Keep existing same-name warning behavior if still applicable | N/A |

</frozen-after-approval>

## Code Map

- `Source/Main.cpp` — combo fill (`filteredInputs_` / `filteredOutputs_`), `openSelectedPorts()`, prefs load/save keys, log strings, inquiry “not open” message
- `README.md` — usage steps and Host↔Simulator ASCII diagram

## Tasks & Acceptance

**Execution:**
- [x] `Source/Main.cpp` -- Wire From→MidiInput / To→MidiOutput; keep From above To in layout; fix logs and inquiry error; add one-shot swap migration + flag -- local referential
- [x] `README.md` -- Update usage step and diagram for crossed host↔simulator ports -- docs match UI

**Acceptance Criteria:**
- Given the Options window, when it is laid out, then **MIDI From** appears above **MIDI To**.
- Given a working host editor with synth-centric From=Port2 / To=Port1, when the simulator uses local From=Port1 / To=Port2, then Device Inquiry still receives and replies.
- Given pre-change saved prefs (From=output ID, To=input ID), when the app launches once after upgrade, then ports reopen correctly without manual re-pick and migration does not re-swap on later launches.
- Given README usage section, when a developer follows it, then they configure host Out→simulator From and host In←simulator To.

## Spec Change Log

## Design Notes

**Golden wiring (after change):**

```
Host editor Out  -->  Simulator "MIDI From"  (MidiInput)
Host editor In   <--  Simulator "MIDI To"    (MidiOutput)
```

Matrix-Control keeps synth-centric labels; correct pairing is therefore crossed port picks between the two apps.

**Prefs migration:** keep key names `midiFromIdentifier` / `midiToIdentifier`. Add bool e.g. `midiPortsLocalReferential` (false/absent = legacy). On load when flag false: swap the two string values, set flag true, then continue. Do not swap enabled input/output filter lists.

## Verification

**Commands:**
- `cmake --build --preset macos-debug-arm64` -- expected: build succeeds

**Manual checks (if no CLI):**
- Open Options: **MIDI From** is above **MIDI To**; From lists inputs only; To lists outputs only.
- With Virtual MIDI Port 1 on From and Port 2 on To (and Control still From=2 / To=1), inquiry log shows listen on From / reply on To and successful reply.
- Quit and relaunch once: same ports restored; second relaunch still correct (no double-swap).

## Suggested Review Order

**App-local From/To wiring**

- From combo filled from MidiInput devices; To from MidiOutput.
  [`Main.cpp:456`](../../Source/Main.cpp#L456)

- Open listen on From, reply on To; logs match new meaning.
  [`Main.cpp:534`](../../Source/Main.cpp#L534)

**Prefs migration**

- One-shot swap when `midiPortsLocalReferential` is absent/false.
  [`Main.cpp:302`](../../Source/Main.cpp#L302)

**Docs**

- Host Out→From / In←To, plus Control cross-wiring note.
  [`README.md:24`](../../README.md#L24)
