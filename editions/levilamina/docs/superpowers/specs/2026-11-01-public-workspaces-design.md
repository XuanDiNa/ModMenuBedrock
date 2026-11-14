# Public plugin workspaces and text controls

## Authority and delivery

The user approved the proposed public asynchronous workspace direction and asked
that it be reusable by plugins other than BedrockTool. Their instruction to
continue routine implementation through a testable package replaces intermediate
design/plan approval pauses. Work inline, without subagents, installation, game
control, publishing, changes to other plugins, or modification of accepted
release files. This delivery is ModMenu plus a real opt-in example provider and
SDK; it does not claim that the existing BedrockTool binary is already adapted.

Use the standalone ModMenu checkout on a feature branch. Windows x64; preserve
the existing independently guarded 1.26.45.01, 1.26.51.01 and 1.26.52.03 profiles.
No new game RVA, texture ABI, resource pack, LeviLamina or helper process.

## Generic, additive contracts

1. Settings description v3 adds `text` to the v2 controls. v1/v2 schemas and
   meanings stay unchanged. Existing legacy text adapters stay private/unchanged.
2. Workspaces use a separate version-1 JSON declaration, supplied explicitly by a
   loaded plugin to a versioned copy-buffer C API. No plugin IDs, page names,
   account operations, paths or network behavior are hardcoded in ModMenu.
3. A plugin can offer passive Configure, a live Workspace, both or neither.
   Registration does not load a DLL. Loaded-only catalog and exact manifest
   folder/entry/version matching still apply. An invalid workspace does not
   disable ordinary settings or another plugin.
4. SDK declarations use fixed-width integers, opaque nonzero session tokens,
   caller-owned UTF-8 buffers and explicit status codes. No STL/Go pointers,
   allocator transfer, foreign callback or cross-module exception contract.

## Text and settings

Public text fields declare `minBytes` and `maxBytes` (0..4096), default and label.
Optional `validation` is `none` or `identifier`; identifier accepts ASCII
letters/digits, underscore, dash and dot, excluding empty, dot and dot-dot.
Values remain JSON strings, preserving regex backslashes and path separators.
Server address, regex and filesystem semantics belong to the provider, not the
menu: editing never performs DNS, reads a path or executes a regex/script.
Provider-specific validation errors can be published against workspace fields.

The shared editor supports valid UTF-8, code-point-safe caret movement and
deletion, Home/End, Shift selection, Ctrl+A/C/V, horizontal text scrolling,
and length errors without silently truncating the user's value. IME committed
text follows the existing native input route; composition UI is not promised.
Clipboard access runs on the menu worker, not rendering. Paste responses carry
an edit generation and cannot land in a closed or different editor.

Enter commits an edit; Escape cancels the active edit before returning. Settings
Apply/Cancel retain transactional backup, stale-source detection and preservation
of unknown config members. Saving a setting is not an operational command or
proof of a companion applying it.

## Workspace declaration

Root: `schemaVersion:1`, `plugin:{id,entry,versions}`, `pages`.
Pages have stable ASCII `id`, localized `label`, and ordered `controls`.
Controls: `text`, `boolean`, `action`, `status`, `progress`, `list`, `image`.
Every control has a stable unique `id` and localized `label`; text uses the
same byte limits/validation as settings; action can declare a localized
`confirmation`; list can declare `selectAction` and `emptyText`.
At least English, Traditional Chinese and Simplified Chinese captions are
supported using the existing language selection/fallback convention.

An action's ID is an opaque provider command. A list selection submits only its
stable row ID and declared select action, never an array index as identity.
Forms remain local drafts while state refreshes arrive. Explicit actions include
a bounded copy of that page's form values and the state revision observed by the
user. No implicit Apply or command on open, render, navigation, status refresh,
reconnect or page close. Tab/arrow navigation is local; Enter/Space activation
uses the same validation/confirmation path as a paired primary click.

## Mailbox and state

The provider worker registers, publishes complete validated snapshots, polls
commands and unregisters. ModMenu never calls plugin code. Registration verifies
the owner address belongs to the matching already-loaded DLL and the real
manifest. It pins that DLL for process lifetime so a published owner cannot be
unloaded under consumers. This is cooperative in-process API validation, not a
security boundary against malicious native code in the same process.

State: `protocolVersion:1`, monotonically increasing `revision`, and `controls`
keyed by declared ID. Each value can contain localized `text`, `error`, `reason`,
`enabled`, `busy`, typed `value`, progress `value/total`, bounded list `rows`, or
image `width/height/pixels`. Unknown controls/properties, wrong types, duplicate
keys, invalid UTF-8 and over-limit state are refused atomically. A failed
publication retains the last valid snapshot. A provider must publish an initial
snapshot before actions become available.

Commands: `protocolVersion`, `session`, strictly increasing `requestId`,
`revision`, `page`, `action`, `values`, optional `rowId`. The FIFO is bounded;
queue full is visible and does not overwrite an earlier operation. Each action
allows only one queued/in-flight request until the provider publishes its
terminal result. Poll with a short buffer reports the required size without
dequeueing. Accepted delivery is not job success; result states are
`succeeded`/`failed` with request ID and optional localized message. No automatic
retries; providers must validate revision, current business state and arguments.

Page closure only discards local UI/confirmations. It never sends Stop or
unregisters. Explicit unregister invalidates the token, pending commands and
state. Old-token messages cannot affect a later registration. Missing providers
are shown unavailable without blocking the UI. Publication snapshots and command
copies are bounded; JSON parsing and identity checks occur outside render locks.

## Resource bounds and rendering

At most 16 providers, 8 pages/provider, 64 total controls/provider, 64 KiB
declaration, depth 16. IDs: ASCII letters/digits/underscore/dash, 1..64 bytes;
identity follows existing settings limits. State: 256 KiB, 128 rows/list,
256 total rows/provider, labels 160 bytes, paragraphs 1024 bytes. A command is at
most 32 KiB; at most 16 queued/in-flight requests/provider. All limits refuse
overflow, not silently replace/drop commands. Lists show explicit empty/error
states; scroll is bounded and selection uses stable row IDs.

Raw image controls accept RGBA8 pixels encoded as uint32 RRGGBBAA, 1..64 wide,
1..48 high, at most one image/provider. No URLs, file paths, compressed image
decoder, texture handles or GPU ownership cross the interface. Convert to
bounded horizontal rectangle runs before rendering; at most 3072 pixel runs.
This is a low-resolution map/preview control, not full-resolution image or 3D
skin rendering. Snapshots are immutable to the renderer; newer states replace
older ones, and provider updates should be no faster than 4 Hz.

## Privacy and compatibility

Never put passwords, access/refresh tokens, cookies or pack keys in descriptors,
ordinary settings, command arguments, status or logs. ModMenu has no secret field
type and does not log workspace payloads. Providers own protected credential
storage and explicit official authentication workflows. Device-login status may
be shown transiently by a provider, but is not persisted by ModMenu.

Existing game eligibility remains main/pause only; no new HUD opening or hooks.
The existing native entry checks and reviewed hook ownership stay fail-closed.
An older BedrockTool binary pins the old ModMenu hash; do not weaken that guard
or promise compatibility. It needs a separate reviewed provider adaptation.

## Verification and acceptance

Test first: v3 string refusal before support; UTF-8 edits/caret/selection and
length validation; temporary-directory transactional save; pure parser/broker
registration, invalid snapshots, revisions, queue saturation, short buffers,
acknowledgement, stale token, dangerous confirmation, no action from reads or
draws, list selection and page close; real exported C ABI and runnable provider.
Run all existing Debug/Release suites plus new suites and native exact-image
checks where original images are available. Review changed native input routing,
PE imports/exports and frozen artifact checksums. Package an additive candidate
and SDK/example separately, preserving accepted 0.5.0 files and installed files.

Manual acceptance remains required per exact game build for text/paste,
workspace navigation/actions/confirmation, asynchronous progress, page-close
job continuity, map rendering and old-plugin input coexistence. Offline tests
and example jobs prove interface behavior, not real login/network integration.
