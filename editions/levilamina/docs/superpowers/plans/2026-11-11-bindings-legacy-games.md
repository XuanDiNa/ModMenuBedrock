# ModMenu legacy-game compatibility verification plan

**Goal:** Deliver only ModMenuBedrock for Minecraft 1.26.45.01 and 1.26.51.01,
preserving its public keyboard-binding UI and the existing 1.26.52.03 profile.

**Architecture:** Revalidate the frozen ModMenu source and existing independent
native profiles against exact original/launcher images. Preserve exact-version
admission, address/layout authority, and hook ownership checks. If the frozen
DLL already contains these profiles, deliver that DLL unchanged after verification.

**Scope correction:** The user explicitly requested ModMenu only. Stop the
earlier other-plugin porting branches; retain isolated candidates without
changing, building, packaging, or installing them. Work inline in a frozen
ModMenu source copy. No installation, game launch/input, publishing, EXE edits,
inferred RVA deltas, or relaxed guards.

- [x] Preserve exact source/DLL archive identities and image identity records.
- [x] Run fresh Debug/Release ModMenu suites and exact-image mutation/refusal checks.
- [x] Validate the delivery DLL's PE, exports, unsupported-host behavior, and source provenance.
- [x] Freeze one ModMenu-only three-version package with SHA-256, sizes, preserved
  settings, and focused manual tests on 26.45 and 26.51.

Offline verification and manual gameplay acceptance remain separate. Missing
native evidence must be reported explicitly rather than labelled compatible.
