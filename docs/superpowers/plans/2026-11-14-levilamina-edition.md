# Independent LeviLamina Edition Implementation Plan

**Goal:** Deliver a standalone ModMenuBedrockLL native client package for LeviLamina 26.60.7 / Minecraft 1.26.60.05.

**Architecture:** Reuse frozen test8 UI and verified 26.60 native adapter. Replace injection-time startup with official LL registration, shared allocator and lifecycle; retain ownership checks and process-lifetime hooks.

**Tech Stack:** C++20, xmake, clang-cl, LeviLamina 26.60.7, MinHook 1.3.4.

**Spec:** `docs/superpowers/specs/2026-11-14-levilamina-edition.md`.

## Global constraints

No changes to vanilla project, EXEs, installed plugins, other plugin projects or GitHub. No guessed addresses or normalized identity admission. No gameplay claims from offline tests.

## Tasks (execute inline as requested)

- [x] Extract frozen source into independent root; record provenance and official docs.
- [x] Configure official client SDK, MD runtime, registration and link rule.
- [x] Add failing exact-60 tests: refuse 45/51/52, admit original/GUI/console reviewed images, reject arbitrary subsystem/code/header edits.
- [x] Restrict native descriptors to 60 and add exact console hash, preserving entry guards.
- [x] Add lifecycle tests: prepare once during load, start once during enable, duplicate edition refusal, failure passthrough, no unsafe unload, disable joins worker and deactivates UI.
- [x] Replace DllMain worker with `LL_REGISTER_MOD`; use official allocator implementation via link rule. Keep synchronous early definitions during load and worker for catalog/files/clipboard.
- [x] Preserve UI/Workspace API, change self identity and manifest to distinct LL edition; document provider migration.
- [x] Build/test Debug and Release, actual DLL ABI/allocator host checks, full UI regression suites and exact images.
- [x] Package DLL, manifest, configuration, SDK and sources; verify SHA-256/size/package whitelist and protected files. Deliver installation and gameplay steps.

## Delivery evidence

Frozen candidate: `releases/ModMenuBedrockLL-0.1.0-test1-ll26.60-20261114-110352/`.
Debug and Release each passed 28 offline suites. Runtime ZIP: 15 files; source ZIP: 189 files; integration SDK: 40 files. Final PE identity, package hashes and protected-file checks are recorded in `ARTIFACTS.json` and `local-audit/delivery-verification.json` in that directory. Source ZIP was frozen before this checklist was marked delivered. Manual gameplay acceptance remains pending; no installation or publication occurred.

## Player image-order follow-up: test2

- [x] Correlate supplied disk report with the frozen test1 DLL and official LL runtime; distinguish disk evidence from live-memory attribution.
- [x] Reproduce the complete alternate PreLoader-first/LeviLauncher-second console image from the exact original and official PeEditor; match the player's complete SHA-256.
- [x] Audit all eight original section headers/bytes, original entrypoint, 88 original DLL descriptors and 1,012 original imported symbols/IAT RVAs.
- [x] Observe nine new positive admission failures against old source, then add only the reviewed GUI/console pair via an immutable descriptor span.
- [x] Review runtime changes inline against frozen test1: only NativeProfile, NativeCompatibility and version strings differ; no addresses, guards, layouts, hooks, UI or provider contracts change.
- [x] Correct verification harness argument forwarding and explicitly bind the isolated xmake project/file so fresh tests cannot accidentally resolve the parent project.
- [x] Run Debug and Release: 28 suites each, 57 LL-native checks and 20 lifecycle checks each; integration/schema/resource-pack audits pass. Both reports require private menu and launcher-order fixtures.
- [ ] Freeze and verify a new test2 delivery and configuration-preserving update archive. Keep test1, installed DLLs/configuration, vanilla project and EXEs intact.

The supplied two-plugin ZIP confirms five ModMenu test1 disk-identity refusals
and matching-PID music-plugin host-admission refusals. Music is outside this
ModMenu-only change and has not been modified. All test2 results above are
offline; gameplay acceptance is pending.
