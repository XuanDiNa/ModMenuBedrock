# Public Workspaces Implementation Plan

> **For agentic workers:** Execute inline using superpowers:executing-plans.
> The user prohibits subagents; do not dispatch them or create another worktree.

**Goal:** Deliver reusable opt-in plugin workspaces and text settings without
breaking passive v1/v2 or relying on BedrockTool-specific code.

**Architecture:** Pure UTF-8 editor and bounded description/state/mailbox model;
versioned C copy-buffer API owned by the ModMenu worker; declarative workspace UI
consumes immutable snapshots and queues explicit commands only.

**Tech Stack:** Windows x64, C++20, existing JSON and native paint/input boundary.

**Spec:** ../specs/2026-11-01-public-workspaces-design.md

## Execution status

Follow-up 0.6.0-test3: split temporary foreground loss from real scene invalidation;
retain verified navigation/drafts and worker state, cancel transient input and
validate the cached provider session before restoration. FocusTests reproduced
16/26 baseline failures; inline review added repeated-focus and unknown-category
regressions, with 29/29 focused checks passing. Clean Debug/Release verification
passed 21 suites/mode and 94 exact-image native checks/mode. Native-boundary and
historical artifact audits passed; a new frozen package is the delivery step.
No BedrockTool or installed plugin edits are authorized by this follow-up.

Follow-up 0.6.0-test2: the supplied Workspace click diagnosis was independently
verified against matching Core/WorkspaceUi/NativeHost hashes. The real mapped-
pointer regression failed four of eleven checks before the minimal Core fix,
then passed all eleven. The target is now part of the full verification gate;
test2 has its own metadata and package and does not replace frozen test1.

Tasks 1-4 are implemented. Focused text/broker/UI/ABI regressions passed,
including full-queue FIFO, clipboard generations, explicit confirmation and
scrolled stable-ID selection. English/TW/CN offline previews were inspected.
The inline review caught and repaired old slider caret clipping, a command NUL
boundary, missing terminal-result feedback and failed-paste selection loss.
The original task checklists below are retained as the planned acceptance scope;
the authoritative completed-run record is the candidate `verification.json`,
with manual game acceptance separately pending per exact build.

Task 5 clean Debug/Release verification passed: 19 suites per mode, six private
exact images, synthetic menu definitions, exported ABI and unsupported host.
The optional extracted vanilla-menu fixture input was not supplied. Packaging
is the final step; keep manual gameplay acceptance separate.
No new game hooks or edits to NativeHost/guard/profile/ASM were needed.
The broker lives in `Workspace.cpp` rather than a separate broker translation
unit; no extra NativeHost input hook was needed.

## Global constraints

- Preserve v1/v2 and exact 1.26.45.01 / 1.26.51.01 / 1.26.52.03 native profiles.
- No LeviLamina, new game RVAs, foreign callbacks, helper process or EXE edits.
- No other-plugin edits, automatic installation/game control or GitHub upload.
- Keep accepted 0.5.0 artifacts immutable; fresh 0.6.0-test1 requires manual tests.
- Descriptor/state/commands are bounded and contain no credentials or script code.

## Task 1 — shared text editor and opt-in settings v3

Files: new `include/ModMenu/TextEditor.h`, `src/TextEditor.cpp`,
`tests/TextEditorTests.cpp`; settings parser/input/save, Configure and View;
new `integration/modmenu.settings.v3.schema.json` and example.

- [ ] Add behavior tests: `ParseSettingsDescription(validV3)` accepts a string
  with `maxBytes:64`; v1/v2 reject it. Insert `A中B`, move left, backspace and
  assert `AB`; select all, paste over limit and assert original value retained.
- [ ] Run the tests before implementation; observe missing capability refusal.
- [ ] Implement code-point boundaries, caret/selection, bounded insertion and
  clipboard intents; add v3 parsing while retaining old legacy text contracts.
- [ ] Save/reload through real temporary config and verify JSON string,
  unknown-member preservation, Cancel and stale Apply.
- [ ] Run existing settings/UI tests plus new editor tests.

## Task 2 — generic workspace contract and broker

Files: `Workspace.h/.cpp`, `WorkspaceBroker.h/.cpp`,
`tests/WorkspaceTests.cpp`, `integration/WORKSPACES.md`.

- [ ] Tests declare pages with text/action/list/image, reject duplicate IDs,
  wrong versions, malformed UTF-8, excessive rows/pixels and unknown controls.
- [ ] Verify no queue changes after snapshots, reads and page closure.
- [ ] Implement strict declaration/state parsing, stable row identities,
  monotonically increasing state revisions, bounded FIFO and result handling.
- [ ] Assert request ID/session/revision/page/action/form/row payloads match
  literal expectations, duplicate in-flight actions refuse, short Poll retains
  the command, unregister invalidates all prior state and requests.

## Task 3 — versioned public C mailbox and real sample provider

Files: `integration/sdk/modmenu_workspace_v1.h`, `src/WorkspaceApi.cpp`,
`src/Plugin.cpp`, new `integration/workspace-example/` and ABI tests.

- [ ] Add DLL-host tests for unsupported API state, invalid ownership,
  fixed-width buffer lengths, null arguments, missing/short output capacity,
  provider handshake and explicit worker command round trip.
- [ ] Implement Register/Publish/Poll/Unregister V1 exports. Validate real loaded
  module path and exact manifest identity on registration; pin accepted owners.
- [ ] Add independently buildable sample plugin with no network/files/login:
  editable name, explicit Start/Stop/Refresh/confirmed reset, progress, a list
  and bounded colored raster. Worker publishes state and processes commands.
- [ ] Keep provider API payloads out of logs; keep test hooks out of shipping DLL.

## Task 4 — shared workspace UI and worker clipboard

Files: `WorkspaceUi.h/.cpp`, Core/View/Configure, NativeHost minimal input route,
Plugin worker, new `tests/WorkspaceUiTests.cpp`.

- [ ] Test real paired clicks, keyboard activation, confirmation cancel/accept,
  stale refresh between press/release, list scroll/selection, editing, Escape,
  focus/resize and job continuity with the broker attached.
- [ ] Add Workspace entry separately from Configure; share layout rectangles
  for rendering/hit tests and immutable snapshot cache for the UI thread.
- [ ] Queue clipboard work by edit generation. Read/write clipboard on worker;
  reject a late paste after editor/page/scene changes.
- [ ] Bound layout/paint at 240x160, 320x180, 640x360 and three locales. Render
  and inspect offline previews; do not describe them as gameplay proof.

## Task 5 — verification, review and delivery

Files: verifier/PE audit for new explicit exports; integration docs, README,
TESTING and candidate metadata; allowlisted candidate package script.

- [ ] Run fresh Debug/Release all existing plus new suites; exact native image
  fixtures, unsupported-host smoke and exported ABI fixture.
- [ ] Inline review because user forbids reviewer agents. Fix safety/behavior
  issues with failing regressions before packaging.
- [ ] Verify unchanged v1/v2 schemas, profile/guard/ASM bytes and old accepted
  release checksums; do not change an old hash audit to call a new build accepted.
- [ ] Package new checksummed DLL-only update, SDK/source examples and optional
  sample DLL separately, with bilingual install/rollback and manual test steps.
- [ ] Report what passed offline and what awaits user testing; do not install.
