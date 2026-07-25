# Deferred Work

- source_spec: `_bmad-output/implementation-artifacts/spec-window-height-logs-resize.md`
  summary: Persist main window height across launches via ApplicationProperties.
  evidence: Resize works in-session but every launch resets to 500×500; user did not ask for persistence in this one-shot.

- source_spec: `_bmad-output/implementation-artifacts/spec-window-height-logs-resize.md`
  summary: Tie outer min height to a content-side minimum for the Status log editor.
  evidence: Chrome (title + menu) already eats space before layout; workable today (~146 px log) but not guarded if chrome metrics change.

- source_spec: `_bmad-output/implementation-artifacts/spec-unknown-device-synth-profile.md`
  summary: Unify DeviceProfile enum ordinals with ComboBox item IDs (or a single shared ID table) to remove dual mapping.
  evidence: Pre-existing 1-based combo IDs vs 0-based enum; three-way converters (`profileFromComboId` / `comboIdFromProfile` / `addItem`) can drift when a fourth profile is added.

- source_spec: `_bmad-output/implementation-artifacts/spec-unknown-device-synth-profile.md`
  summary: Surface or reject corrupt `synthProfileId` prefs instead of silently mapping unknown IDs to Matrix-1000.
  evidence: Pre-existing fallback behavior; `profileFromComboId` default still hides bad or forward-incompatible saved IDs.

- source_spec: `_bmad-output/implementation-artifacts/spec-unknown-device-synth-profile.md`
  summary: Rename asymmetric `kExpectedMember*` constants to an explicit Matrix-1000 name so all three profiles read as peers.
  evidence: Naming asymmetry predates Unknown Device (`kExpected*` vs `kMatrix6*`); cosmetic SSOT clarity, not required for this feature.

- source_spec: `_bmad-output/implementation-artifacts/spec-unknown-device-synth-profile.md`
  summary: Add a scripted or unit check that combo ID 3 encodes member bytes `00 00` with manufacturer `10` and family `06`.
  evidence: Project has no inquiry-reply unit tests yet; wrong switch arm would only show up in a MIDI monitor.
