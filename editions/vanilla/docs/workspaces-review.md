# Public workspaces candidate review / 公用工作頁審查

## 0.6.0-test3 focus suspension

The focus-loss path used to discard `workspaceOpen`, the snapshot and Configure
drafts together with scene invalidation. Core now suspends transient input while
retaining a previously verified Main/Pause origin category. Verified background
sibling callbacks carry no viewport and cannot invalidate or restore navigation.
Actual ineligible frames, category changes, unknown replacement categories or
invalid top viewports still close stale UI. No native pointers are retained.

Workspace suspension cancels press pairing, editing/paste generations and
dangerous confirmation, preserving page/scroll/form values and provider results.
Configure rolls back unfinished slider drags and cancels control capture without
discarding the draft or automatically saving it. Release barriers prevent held
mouse/key input from acting after refocus, including raw keys arriving before
the first physical sample and repeated focus cycles before button release.

Cached provider/session validation runs before Workspace restoration. Worker
snapshot updates and previously queued commands continue; focus changes never
submit Stop, retry or replay. The API, schemas, broker, worker and NativeHost
remain unchanged. No new native profile, hook or guard change is required.

`FocusTests` uses real Core/WorkspaceUi/broker state. The initial 26-check baseline
failed 16 assertions; 29 checks now pass, including two extra regressions caught
in inline review (repeated focus cycles and an unknown replacement category).
The clean Debug/Release gate passed 21 suites per mode, including 94 native
checks per mode against six exact original/launcher images. Private extracted
vanilla-menu fixtures were not supplied; synthetic definitions passed. Shipping
PE remains AMD64/static CRT with exactly six intended public exports.

Release DLL SHA-256:
`91C01317BA63D93ED7A97583FEDD325D2C388E15C5A56100D2BB39FECB544C85`
(957952 bytes). Debug SHA-256:
`4BEB3A10E5367C0BDD1656414252E3202B8D7953D26F1408CC192B327AF86A43`.
The candidate package's fresh `verification.json` and checksum files record the
run and frozen payload. This review does not claim manual gameplay or live
authentication success. Historical 0.5.0, test1 and test2 DLL/ZIP/payload audits
passed with their original hashes; no installed files changed.

本次在同一對話審查，不使用審查子代理。只拆分失焦暫停與真正場景失效；保留頁面、
捲動及已提交背景工作，取消暫態輸入並防止回焦誤觸。原生保護／版本資料與舊介面不變。
BedrockTool 仍需獨立審核並固定新 DLL 雜湊，未自動安裝或發佈。

## Historical 0.6.0-test2 pointer fix

The test2 runtime delta is limited to synchronizing `Menu::pointerX/pointerY`
on Workspace Move before delegation, after the existing scene/focus/viewport
guards. `NativeHost` mapped Down/Up consume those coordinates. The native
adapter, profiles, guards, ABI, WorkspaceUi and broker are unchanged from test1.
No busy/pending/revision/confirmation checks are removed.

`WorkspacePointerTests` links the real Core, WorkspaceUi and broker. It uses
Move followed by Down/Up from the stored pointer, unlike the old direct-coordinate
tests. Before the fix: 11 checks, exactly four failures (pointer, action, page,
Back); after the fix: 11 checks, zero failures. Control cases cover keyboard,
direct-coordinate clicks, ineligible scene, focus loss, small viewport and
observer-only input. The target is included in both modes of `tools/verify.ps1`.

Fresh Debug and Release verification passed: 20 suites per mode, including
94 native checks using the six exact local game images in each mode. Private
vanilla-menu fixtures were not supplied; synthetic definition tests passed.
The shipping PE is AMD64 with static CRT and exactly the same six public exports.

Test2 Release SHA-256:
`AAE7F78FA77AC6BD1071E757590DB56B79FF8CB51AEDFFE5B0BCE735AA801E3D`
(956928 bytes). Debug SHA-256:
`99B5834EC5E51C997D3B8B9486B097A98C725102FE086AD909B01046984E06DF`.

The fresh test2 run and exact DLL hash are recorded in the new package's
`verification.json` and `SHA256SUMS.txt`. The test1 record below is historical,
not the hash or test-suite count of test2. Both old packages remain frozen.
BedrockTool test2 pins the old ModMenu hash; it needs a separately reviewed
matching build before joint testing. No installation or GitHub upload is made.

test2 只在原有場景／焦點／視窗檢查後同步工作頁 Move 座標，不更改原生掛鉤、
版本保護、ABI 或操作安全檢查。新測試使用真正 Core／WorkspaceUi／broker：
修正前 11 項中 4 項失敗，修正後 11 項通過，並納入完整雙模式驗證。
新套件指紋以自身校驗檔為準，下方是保留的 test1 紀錄；人工遊戲驗收仍待完成。

## Historical 0.6.0-test1 review

## Scope

Local **0.6.0-test1** candidate on `codex/public-workspaces`, based on `861e98a`.
Only ModMenu and its public SDK/example change. No installation, Minecraft
launch, other-plugin edits, EXE changes, new game offsets, LeviLamina, external
helper, GitHub upload or modification of accepted 0.5.0 artifacts.

The new API is cooperative and in-process, not an untrusted-code sandbox. Owner
registration checks the already-loaded DLL, sibling directory, manifest entry
and exact plugin version; accepted owners are pinned. Caller memory must be valid.

## Inline review and focused verification

Review was performed inline, without reviewer agents. Checked the parser/model,
C ABI, worker/render boundary, UI/input/clipboard lifecycle, build and packaging.

- New settings v3 uses explicit bounded UTF-8 text. v1/v2 remain unchanged and
  still reject that field type. Real config tests cover transactional save,
  unknown-field preservation, Cancel and changed-source refusal.
- Commands require an explicit paired click/keyboard activation. Reads, paint,
  state publication and page close do not submit commands. A changing snapshot
  cancels a pending mouse press; dangerous confirmation checks its revision.
- Terminal provider results are visible with locale fallback. Worker jobs survive
  page closure. Session IDs/revisions/stable list IDs prevent stale delivery.
- FIFO saturation refuses the seventeenth queued/in-flight request without
  dropping earlier work. Short Poll preserves the command. The maximum command
  buffer includes its NUL terminator; the 32769-byte boundary is refused.
- Clipboard read/write is worker-only for new editors. Domain/generation checks
  reject late paste into a cancelled or different editor. An empty/rejected
  clipboard preserves the selected value and displays an error.
- Existing compact numeric-slider caret clipping was restored independently of
  the new Unicode text path; its 128-check suite passes in the focused run.
- Actual cross-DLL tests use the real example provider and public C API against
  an offline Ready fixture. They check argument/ownership refusal, idle state
  without commands, explicit Start, progress and completion. Fixture exports are
  not present in the shipping DLL.
- Offline previews inspected in English, Traditional and Simplified Chinese at
  compact and normal sizes. They approximate native text metrics, not gameplay.

Focused regression runs and the complete clean verification passed on 2026-11-01:

| Check | Result |
| --- | --- |
| Debug / Release | 19 suites each; zero failures |
| Exact original + launcher images | Six images across all three supported builds; 94 native checks each mode |
| Vanilla-menu definition data | Synthetic tests passed; private extracted fixtures not supplied |
| Shipping PE | AMD64, static CRT, exactly six intended exports; no test exports or LeviLamina |
| Historic 0.5.0 ZIP / DLL | Original SHA256 values and payload checksums unchanged |

New Release DLL SHA256:
`C625470199A829177489FB71F8CAA30FADDAD4C0CC5D5480043AE6DEFD349FBD`
(956928 bytes). Debug SHA256:
`E1036912FA7151A7DD7D123C426C3DED4A8C4A3E3766828C0BC3F7BB7A588E13`.
The package's `verification.json` records the fresh run; packaging also audits
both ZIP trees/checksums and the real optional provider against the C ABI fixture.
No offline result substitutes for manual gameplay acceptance of the new DLL.

## Remaining boundaries

- BedrockTool is not adapted. Its existing hash guard may refuse this candidate;
  preserve that guard and review its provider/native compatibility separately.
- No real authentication, server connection, download, export, script execution,
  live map producer or skin renderer is implemented in the example.
- Image control is a bounded 64x48 RGBA8 raster, not arbitrary GPU texture sharing.
- Page navigation uses previous/next controls; longer lists use wheel scrolling.
  Forms are local drafts, not continuously overwritten by provider refreshes.
- Committed Unicode text/paste and codepoint editing are supported; native IME
  composition UI is not supplied. Selection is logical; caret is shown, without
  a native selection-highlight widget.
- Providers own semantic validation and secure credential storage. Malicious
  native DLLs are outside the security model. No payload logging is added.
- Each exact supported game build still needs manual testing of this fresh DLL,
  especially input coexistence, text/clipboard and asynchronous UI lifecycle.

See [manual acceptance and rollback](workspaces-testing.md) and
[public interface](../integration/WORKSPACES.md).

## 繁體中文摘要

本版只擴充 ModMenu／公用 SDK／示範，沒有改動其他插件、安裝遊戲檔或上傳 GitHub。
審查及測試都在本對話執行。操作只由明確輸入觸發，刷新不執行動作；關頁不中止工作。
已針對舊測試版本預期、狀態刷新後點擊、結果顯示、貼上世代及命令長度邊界補回歸。

主套件保留舊 v1／v2 及原生 Hook／安全檢查，示範 DLL 獨立選裝。完整離線結果以
套件 `verification.json` 為準；三個版本的新 DLL 仍須人工驗收，不把舊版驗收沿用過來。
目前 BedrockTool 還需獨立接入，且不可移除其既有雜湊檢查強制使用。
