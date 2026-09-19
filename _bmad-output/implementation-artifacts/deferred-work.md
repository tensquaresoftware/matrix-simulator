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

- source_spec: `_bmad-output/implementation-artifacts/spec-firmware-inquiry-digits.md`
  summary: Add golden tests for Device Inquiry rev packing (`111`→`20 31 31 31`, `110`→`20 31 31 30`, short lengths, empty→default).
  evidence: No unit harness in this repo yet; packing correctness is only checked via Status log hex and Matrix-Control display.

- source_spec: `_bmad-output/implementation-artifacts/spec-firmware-inquiry-digits.md`
  summary: Confirm whether stock Matrix ROMs always emit three digits (one leading space) vs arbitrary 1–4 digit lengths.
  evidence: Oberheim docs only exemplify version 1.10 as ` 110`; 1–4 digit UI allowed by protocol wording but may over-simulate.

- source_spec: `_bmad-output/implementation-artifacts/spec-firmware-inquiry-digits.md`
  summary: Tighten legacy dotted-prefs migration beyond digit-stripping if exotic saved values appear.
  evidence: `"1.11"`→`"111"` is correct for prior defaults; ambiguous forms like `"10.1"` are unlikely from this app.

- source_spec: `_bmad-output/implementation-artifacts/spec-juce-9-sdk-path.md`
  summary: Guard against a leftover JUCE_DIR env/cache still selecting JUCE-8 while docs say JUCE 9.
  evidence: CMake prefers -DJUCE_DIR / ENV{JUCE_DIR} before JUCE_DIR_MACOS; JUCE-8 still exists on disk beside JUCE-9.

- source_spec: `_bmad-output/implementation-artifacts/spec-juce-9-sdk-path.md`
  summary: Re-audit cpp-juce-standards deprecated-API bullets against JUCE 9 (e.g. Timer vs HighResolutionTimer).
  evidence: Only the version label was bumped; juce::Timer remains a first-class API in JUCE 9.
