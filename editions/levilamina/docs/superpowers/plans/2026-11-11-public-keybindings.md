# Public keyboard bindings implementation plan

> Execute inline using executing-plans; the user has declined subagent execution and authorized routine implementation through a testable handoff.

**Goal:** Opt-in keyboard bindings with Esc unbinding, broad keyboard coverage and exact Ctrl/Shift chords.

**Architecture:** A source-distributed, platform-independent header owns tokens, parsing, matching and capture. ModMenu edits v4 `binding` fields. Each consumer retains its own eligibility checks and action ownership.

**Tech stack:** C++20, xmake, existing JSON transaction layer, Windows input sampling.

**Spec:** `docs/superpowers/specs/2026-11-11-public-keybindings-design.md`.

## Constraints

- Preserve old v1/v2/v3 files and contracts, exact game guards, accepted release artifacts, installed DLLs/configs/logs, and existing unrelated working changes.
- No game launch, injection, installation, publishing, EXE edits, new native addresses, output-key synthesis or companion admission changes.
- The user's continuous-execution instruction supplies implementation authority; routine design choices do not need another confirmation. The detailed spec records these decisions rather than claiming separate written-spec approval.

## Tasks

- [x] 1. Add `integration/sdk/ModMenuBinding.h` and executable parser/matcher/capture tests. Reject Esc/mouse/unassigned keys; prove canonical round trips, exact modifiers, primary-edge order and release barriers. Run tests red, then green.
- [x] 2. Add v4 `binding` parsing and validation without changing legacy descriptors. Exercise real Load/Apply transactions and malformed descriptions in `tests/BindingTests.cpp`.
- [x] 3. Integrate capture and own-open-key chords into Core/Configure/View/Storage. Exercise press/release, raw/sample ordering, focus loss, unbinding, Backspace, Apply/Cancel and interrupted capture. Preserve existing menu/Workspace gates.
- [x] 4. Adopt only user-input shortcut parsing/triggering in current AutoClick, AutoJumpReset v2, ContainerPreview, FakeSneak, FullBright, ToggleSprint and Muonium sources. Give each a local SDK copy for independent builds; migrate descriptors only alongside matching runtime candidates. Preserve output jump mappings and mouse settings.
- [x] 5. Run focused consumer tests and existing project verification/builds. Review changes inline against lifecycle and exact-version constraints. Run ModMenu Debug/Release/native/Workspace checks.
- [x] 6. Freeze new versioned candidate DLLs with opt-in descriptors, source SDK, bilingual usage/testing notes and SHA-256/byte manifests. Do not install or publish. List manual acceptance still required.

## Test examples and interface

`modmenu::binding::Parse("CTRL+SHIFT+K")` yields `{0x4B, Ctrl|Shift}`. `Parse("ESC")`, `Parse("LBUTTON")` and `Parse("CTRL+NONE")` fail. `Token({0xBB,Ctrl})` yields `CTRL+OEM_PLUS`.

`Tracker::Update(binding, sample, eligible)` yields separate `pressed` and `held` states. Starting eligible with the primary already down does not act; a released sample followed by Ctrl then K acts once; K then Ctrl does not act. Focus/config changes impose a fresh release boundary.

`Capture::Begin(sample)` excludes preheld input. `Event(key,down,sample)` returns no candidate for Ctrl-down; K-down with Ctrl held yields `CTRL+K`; LCtrl-down/up alone yields `LCTRL`; Esc yields `NONE`. Mouse is ignored. Capture is discarded on page/focus invalidation.
