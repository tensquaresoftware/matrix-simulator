# Deferred Work

- source_spec: `_bmad-output/implementation-artifacts/spec-window-height-logs-resize.md`
  summary: Persist main window height across launches via ApplicationProperties.
  evidence: Resize works in-session but every launch resets to 500×500; user did not ask for persistence in this one-shot.

- source_spec: `_bmad-output/implementation-artifacts/spec-window-height-logs-resize.md`
  summary: Tie outer min height to a content-side minimum for the Status log editor.
  evidence: Chrome (title + menu) already eats space before layout; workable today (~146 px log) but not guarded if chrome metrics change.
