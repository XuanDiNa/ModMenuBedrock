# Binding candidate review

Reviewed inline under the user's no-subagent constraint. The scope is public
binding syntax/capture and existing plugin shortcut parsing/trigger logic.

- v5 is opt-in. Existing v1/v2/v3/v4 schema bytes are preserved. Native host
  appends only a read-only VK_LMENU sample to existing key-event metadata. The
  verifier checks that exact byte replacement; profiles, guards and pointer
  bridge source remain byte-identical to the prior base.
  The Workspace ABI exports are unchanged.
- Checked Escape-to-NONE, Backspace, standalone modifier release, pending
  modifier captions, raw/sample ordering, capture interruption and held-key
  barriers. Fixed Configure closing on a chord's bare primary key.
- Checked exact Ctrl/Shift/LeftAlt masks, late-modifier order, toggle repeats and hold
  release. Kept existing plugin eligibility and native gameplay gates. A missing
  ToggleSprint observation clears the new binding tracker.
- Numeric and numeric-string legacy values keep their old VK meaning. Digit
  captures/default resets use VK_XX where necessary; unedited JSON number/string
  representation and unrelated configuration values are preserved.
- Added an SDK macro guard for a translation unit that includes two physical
  copies. All delivered consumers use an identical SDK copy.
- Muonium is built from frozen test4 sources. Only reviewed metadata,
  binding and test files plus the SDK were applied; concurrent native work in
  the active player project was excluded.
- One local launcher-modified 26.51 image differs from ContainerPreview's
  existing approved hashes. Its safe rejection is tested. Original exact images
  remain checked; no guard or compatibility range was broadened.

Baseline test4: Debug and Release each passed 22 ModMenu suites, including 360 binding checks,
94 exact-image checks, 29 focus checks and 11 Workspace pointer checks. Schema
agreement covers 1,777 cases. Consumer verification passed 81 offline commands.
Private vanilla-menu fixtures were unavailable; synthetic fixtures ran instead.
None of this establishes manual gameplay acceptance of the new DLL hashes.

Reproduce with `tools/verify.ps1`, `tools/verify_bindings.py`, and the consumer
verifier using the frozen player source location and locally owned game images.
Freeze this candidate with `tools/package_bindings.py`; the historic Workspace
packager retains its old candidate identity and should not package these DLLs.

## Left Alt candidate review

SDK contract 2 and settings v5 add the side-specific LeftAlt modifier. v4 values
retain their original range and readable CTRL+LALT primary-key spelling. The
menu tests exercise raw-first and sampled input, full chord persistence, exact
RightAlt refusal and standalone LeftAlt release capture. Consumer packed masks
now retain all three bits. Native input metadata is read-only and adds no hooks.
Current candidate passed Debug and Release, each with 22 suites: 431 binding
checks, 94 exact-image checks, 29 focus checks and 11 Workspace pointer checks.
Schema/parser agreement covers 3,525 cases against both frozen v4 and new v5.
Consumer verification records 88 successful offline commands, including the
final AutoClick identity rebuild and isolated player DLL build. Private menu
fixtures remain unavailable; synthetic fixtures ran. Gameplay is still pending.

## Matching installation follow-up

Read-only attribution found the installed menu already matches test5-leftalt,
while all seven consumers still use legacy DLLs and single-key descriptions.
No menu runtime change is justified. The new flat 26.52 bundle reuses that exact
menu DLL and six frozen consumer DLLs. It replaces the earlier 0.6 player
candidate with 0.7.0-test2-leftalt based on the exact installed 0.7.0-test1 source
archive. The player changes nine binding/metadata/test files and adds only the
canonical SDK; every other original source file, including native and Java,
is byte-identical. Active player development is untouched.

The package fixture checks actual descriptions for all fourteen activation
fields through mouse selection, all seven modifier masks, Apply and reopen;
898 checks pass. Player Debug and Release each pass the eight existing test
executables and the production DLL unsupported-host check; three exact-image
static verifiers pass in Release. The read-only bundle checker requires the
matching DLL hash/size, manifest and schema-v5 sidecar, with duplicate loader
copies reported. This narrows the remaining manual test to installing the
matching sets and checking capture plus the existing action in each plugin.
