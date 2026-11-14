# Public keyboard bindings and cooperative plugin adoption

## Status and authority

Implementation scope recorded under the user's continuous-execution authority;
offline verification has passed and gameplay acceptance remains pending.
The user requests every observable keyboard key except Escape and mouse buttons,
plus Ctrl/Shift combinations. They permit changes to ModMenu's public binding
contract and existing plugins' shortcut parsing/trigger logic. Work inline;
do not auto-install, launch/control Minecraft, publish, change game executables,
weaken exact-image guards or replace frozen artifacts.

This is an architectural change because independent plugin consumers must agree
on syntax, capture semantics and triggering. A UI-only change cannot implement it.

## Approaches

1. Recommended: opt-in settings v4 binding control plus a small source-distributed
   header-only keyboard parser/matcher. ModMenu owns editing/capture; each plugin
   retains its own input sampling, eligibility and action execution. No new DLL
   exports, native hooks, network path or mandatory runtime dependency.
2. Extend legacy key token allowlists in place. This is smaller superficially but
   can write strings older runtimes reject, changes existing schemas' meanings,
   and still duplicates each plugin's parser. Reject this approach.
3. Centralize plugin action dispatch inside ModMenu. This could arbitrate global
   conflicts but needs a new action/lifecycle ABI and makes plugins depend on
   ModMenu at runtime. It is unnecessary for the requested binding feature.

## Public contract and compatibility

- Add settings-description v4 and a distinct `binding` field type. Keep v1/v2/v3
  files and semantics unchanged. A runtime must opt in before its field is shown
  as an unrestricted/chord-capable binding. Do not infer support from plugin name.
- Store a bounded canonical string: `NONE`, `K`, `CTRL+K`, `SHIFT+K`,
  `CTRL+SHIFT+K`. Modifier order is canonical; Ctrl and Shift are side-neutral.
  Left/right-specific standalone keys remain representable, e.g. `LCTRL`.
- Provide stable names for letters/digits, navigation/editing, F1-F24, numpad,
  OEM punctuation, lock keys, Windows/menu keys and observable media/browser keys.
  Main-row and numpad keys remain distinct. Use a documented `VK_XX` fallback for
  assigned observable Windows keyboard keys without a friendly name, never for
  mouse, Escape, reserved/unassigned or sentinel codes. OEM_PLUS is distinct from
  the string separator. Bound total encoded bytes and reject malformed tokens.
- Only Ctrl and Shift are chord modifiers in this increment. Alt/Windows keys
  can be standalone primary keys if observable; Alt/Win chord syntax is excluded.
  Chords contain at most those two modifiers and one non-Ctrl/Shift primary key.
- Old single-key/numeric config values retain their readable representation.
  A migrated plugin accepts its previous valid syntax alongside the new syntax;
  no blanket config rewrite on load. New bindings use the canonical string.
  Only updated, version-declared descriptors offer the new editor.
- Existing legacy descriptors can retain their documented Backspace-as-unbind
  behavior until migrated. In the new editor, Backspace is a real bindable key;
  only Escape chooses NONE. Never silently change an unupdated consumer.
- Preserve root config unknown fields, transactional Apply/Cancel, backups,
  source-change refusal and hot-reload semantics. Reset restores the declared
  default; no new default key or enabled state is chosen for a plugin.

## Capture and display

- Clicking a binding starts capture and displays pending modifiers. Ctrl/Shift
  do not immediately finish capture: press a primary key to capture the chord;
  if a modifier alone is pressed and released without another key, capture its
  side-specific standalone token. Ambiguous modifier-only multi-key attempts
  remain in capture with a concise hint rather than inventing a chord.
- Escape clears to NONE without leaving Configure or saving until Apply. Ignore
  mouse buttons as candidates; interacting with the UI is not a binding.
- Clear capture candidates on focus loss, scene invalidation, provider removal,
  Cancel or page closure. Preheld keys cannot become new candidates. Capture
  completion cannot toggle a feature or close Mods with the same held key.
- Display human-readable captions in the existing game-language mechanism,
  keeping the saved token language-neutral. Show invalid/unsupported values;
  never truncate or fall back to an old/default key when parsing fails.

## Runtime matching

- New opt-in bindings match the exact Ctrl/Shift state, excluding the primary
  itself when the primary is a standalone modifier. Thus CTRL+K does not match
  K alone or CTRL+SHIFT+K. A standalone primary modifier is not a chord prefix
  deferred globally at runtime; only binding capture defers its selection.
- A press/toggle shortcut fires once on a fresh primary-key down while required
  modifiers are already held. Pressing modifiers after an already-held primary
  does not synthesize a new press. Holding/repeating does not retrigger.
- A hold shortcut (such as container preview) is active only while the complete
  chord and the plugin's existing eligibility conditions are satisfied. Release
  any required key to end the hold. No toggle/hold behavior is interchanged.
- Retain each consumer's foreground, world, screen, loading, death and input-
  ownership gates. Focus/eligibility/config changes re-prime edge state; no fresh
  action is generated on return from preheld input. No OS/system input capture,
  system-shortcut suppression or fabricated modifier input is added.
- Duplicate or overlapping bindings may be diagnosed but are not automatically
  remapped or globally suppressed. Independent plugins keep their ownership.
  In particular a standalone Ctrl shortcut can overlap another Ctrl chord.
- Windows-reserved combinations and firmware-only Fn cannot be guaranteed;
  support means observable keys, not intercepting the operating system.

## Adoption scope and separation of concerns

Start with ModMenu's own open binding, then existing user-input shortcuts in
AutoClick, AutoJumpReset v2, ContainerPreview, FakeSneak, FullBright, ToggleSprint
and MuoniumPlayerBedrock. Verify active source/config/descriptor identity before
editing each project; archives, vendored upstream code and frozen packages are
not implementation targets. Build candidates only for each project's existing
verified exact game versions; no new game-version support is inferred.

Do not reinterpret AutoJumpReset's emitted jump key as an activation chord, or
change AutoClick hotbar maps/UI-protection lists, MouseTweaks gestures, gameplay
actions, damage attribution, movement policy, rendering or network behavior.
Keyboard-only exclusions apply to the new binding editor, not to unrelated
physical-mouse gameplay settings or legacy input-output mappings.

BedrockTool dependency admission is not shortcut parsing. If a new ModMenu hash
requires its admission update for joint Workspace testing, obtain separate
approval and retain exact hash/size checks; do not silently broaden that scope.

## Verification and delivery

1. Test the pure parser/canonicalizer and all assigned allowed/refused key codes;
   round-trip OEM_PLUS, numpad, standalone modifiers, every chord shape, NONE,
   malformed input, size bounds and legacy numeric/key syntax.
2. Exercise real capture: modifier press/release, chord capture, Backspace,
   Escape unbinding, ignored mouse, Apply/Cancel/reset, reload, invalid config,
   focus/scene interruptions, held-key barriers and raw/sample ordering.
3. Test runtime toggle/hold behavior, exact modifiers, primary rising edges,
   no action during UI capture, config changes and late modifier order. Use
   consumer-specific eligibility tests; shared matcher tests are not sufficient.
4. Run existing ModMenu Debug/Release/native/profile/Workspace gates unchanged,
   plus each adopted plugin's focused/full verifier. Public schemas v1/v2/v3
   remain byte-identical. Review inline, with no reviewer agents.
5. Freeze new checksummed candidates and a source SDK/example with bilingual
   install/rollback and manual acceptance steps. Keep configs, installed files,
   prior artifacts and logs untouched. Report exact DLL hashes and sizes.
   Separate offline success from manual gameplay acceptance per exact build.

## Scope summary / 範圍摘要

新增選用 v4 `binding` 與共用原始碼解析／比對工具；舊 v1／v2／v3 不變。
新制只用 Esc 解除綁定，Backspace 可綁；允許一般單鍵、Ctrl＋鍵、Shift＋鍵及
Ctrl＋Shift＋鍵，並精確比對修飾鍵。組合鍵只擴充使用者觸發快捷鍵，不改自動輸出
跳躍鍵、滑鼠手勢或其他功能。Ctrl／Shift 單獨綁定在綁鍵介面中於放開時確認。
先更新 ModMenu，再逐一接入上述插件，不自動安裝或發佈。

## Approved Left Alt extension

The user subsequently confirmed Left Alt plus other keys. Settings v5 explicitly
opts into SDK contract 2, adding LeftAlt bit 4 and canonical CTRL+SHIFT+LALT+KEY
order. Frozen v4 remains Ctrl/Shift only. LeftAlt is side-specific; RightAlt and
AltGr do not satisfy it. LeftAlt standalone capture finishes on release. Existing
CTRL+LALT remains readable with LeftAlt as the primary. The only native host
change is a read-only VK_LMENU sample appended to existing key metadata; all
profiles, native guards, hooks and scene/focus authority remain unchanged.
