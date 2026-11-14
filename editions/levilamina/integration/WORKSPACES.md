# Workspaces and public text / 工作頁與通用文字

**Independent LL edition:** resolve `ModMenuBedrockLL.dll`. The API below is
unchanged; the ModMenu host now requires LeviLamina 26.60.7. The example provider
is an offline ABI fixture, not an installable LL native mod. For a real LL
provider, use the official `LL_REGISTER_MOD` lifecycle and shared allocator;
start its worker during `enable`, not from the fixture's `DllMain`. See
[edition migration](../docs/levilamina-provider-migration.md).

## English

**ModMenuBedrock 0.6.0-test3 — experimental, opt-in.** The unchanged v1/v2
settings contracts remain supported. Settings v3 adds plain UTF-8 `text` fields;
the separate workspace protocol v1 adds live pages. Neither requires LeviLamina.
This is a cooperative in-process interface, not a sandbox for untrusted DLLs.

### Passive text settings

Use `schemaVersion: 3` in the existing `modmenu.settings.json`, with the same
identity, file, transaction and translation rules as v2. Add fields such as:

```json
{"path":"/server","type":"text","default":"localhost:19132",
 "minBytes":0,"maxBytes":256,"validation":"none","label":"Server address"}
```

`minBytes`/`maxBytes` count UTF-8 bytes, not characters (maximum 4096).
Optional `validation` is `none` or `identifier`; identifier permits ASCII
letters/digits/underscore/dash/dot, excluding empty, `.` and `..`. Regexes,
exclusion lists and paths are literal single-line strings; the plugin owns their
semantics. Editing never resolves addresses, runs regexes/scripts or reads paths.
No password/token field exists. Use the [v3 editor schema](modmenu.settings.v3.schema.json).

Edit with Left/Right, Home/End, Shift selection, Ctrl+A/C/V and Backspace/Delete.
Committed Unicode text and paste are supported; native IME composition UI is not
provided. Enter commits, Escape cancels that edit. Apply saves transactionally;
Cancel discards. A successful JSON save is not a companion acknowledgement.
Empty, unreadable or over-limit clipboard content leaves the selected text intact
and displays a paste error; invalid paste is never silently truncated.

### Dynamic workspace declaration

See the [real provider source](workspace-example/Example.cpp) and
[C ABI header](sdk/modmenu_workspace_v1.h). A loaded native plugin explicitly
registers a UTF-8 declaration containing:

```json
{"schemaVersion":1,
 "plugin":{"id":"Example","entry":"Example.dll","versions":["1.0.0"]},
 "pages":[{"id":"jobs","label":{"en_US":"Jobs","zh_TW":"工作","zh_CN":"工作"},
 "controls":[{"id":"start","type":"action","label":"Start"}]}]}
```

IDs are stable ASCII letters/digits/underscore/dash. Plugin folder, loaded DLL
basename and exact manifest version must match. Register an address of your own
DLL function. The DLL and ModMenu must be in sibling plugin folders. Registration
pins the owner for the process lifetime; this API is not an unload/reload facility.
No signature verification or privilege isolation between native plugins is implied.

To build the optional example from this source checkout:

```powershell
xmake f -P . -y -m release
xmake build -P . -y -j 2 ModMenuWorkspaceExample
```

It uses the SDK header and the vendored MIT-licensed nlohmann/json dependency;
do not confuse it with the older descriptor-only author templates.

Controls:

| Type | Description / declaration additions |
| --- | --- |
| `text` | Local string draft; required `minBytes`, `maxBytes`; optional `validation` |
| `boolean` | Local Boolean draft |
| `action` | Explicit command with its ID; optional localized `confirmation` |
| `status` | Localized status/error/reason |
| `progress` | Unsigned `value`/positive `total`, value no greater than total |
| `list` | Stable selectable row IDs; optional `selectAction`, `emptyText` |
| `image` | RGBA8 uint32 `0xRRGGBBAA` raster; maximum 64×48; no path or URL |

Every control requires `id`, `type`, `label`. Labels can be English strings or
objects containing required `en_US` and optional `zh_TW`/`zh_CN`; missing text
falls back to English. The menu follows the selected game language. A plugin may
provide both Configure and Workspace without coupling their drafts.

### C ABI and worker ownership

Resolve the four exact `MMB_Workspace*V1` export names shown in the SDK from an
already-loaded `ModMenuBedrock.dll`; never load another copy or call from DllMain.
Return codes: `0` OK, `1` Poll needs a larger output, `-1` invalid arguments,
`-2` menu not Ready, `-3` refused/invalid session or data. A successful registration
returns a nonzero session; a duplicate active plugin ID is refused.

1. Register once on a worker after menu status `MMB_GetStatus()==3`.
2. Publish complete immutable snapshots. Publish never runs an action.
3. Poll your session on the plugin worker and execute explicit commands there.
4. Publish terminal request results. Unregister only on provider shutdown, never
   because its page closed. Closing a page must not stop a job.

All input buffers are copied during the call. Poll's required length includes
the NUL terminator; zero means no pending command. Querying with null/zero or
using a short buffer never removes a command. No Go pointer, C++ object, foreign
callback, allocator ownership or exception crosses the ABI. Providers must keep
their own runtime working when the menu is unavailable.

Example snapshot:

```json
{"protocolVersion":1,"revision":1,
 "controls":{"start":{"enabled":true,"busy":false},
 "progress":{"value":2,"total":10},
 "servers":{"rows":[{"id":"server1","label":"Example","text":"Ready"}]}},
 "results":[{"requestId":7,"state":"succeeded","message":"Started"}]}
```

Only declare controls you actually use; this example's `progress` and `servers`
must appear in the registered declaration. Snapshot control properties are
`value`, localized `text`/`error`/`reason`, `enabled`, `busy`, `total`, `rows`,
`width`, `height`, `pixels`, with types constrained by the control. Rows contain
`id`, localized `label`, optional localized `text`/`reason` and Boolean `enabled`.
Unknown properties/IDs, duplicate JSON members, invalid UTF-8, wrong types or
over-limit payloads refuse the entire snapshot. Keep the last valid state.

Revisions must strictly increase. A deliberate action submits:

```json
{"protocolVersion":1,"session":1,"requestId":7,"revision":12,
 "page":"jobs","action":"start","values":{"name":"Example"}}
```

List selections also contain `rowId`, never an unstable array index. Commands
carry only the form values on the originating page. Providers must revalidate
arguments and business state; `revision` is the state the UI used, not a lock on
the worker. Busy/disabled actions refuse. Each action is blocked until its
previous request gets `succeeded` or `failed` in a published `results` array.
Delivery is not completion. There are no automatic action retries/replays, and
rendering, page switches, reads and status refreshes never enqueue commands.

Temporary focus loss suspends input, not jobs or valid navigation. ModMenu keeps
the current page, scroll and committed local form values while cancelling press
pairing, key capture, unfinished editors/paste and dangerous confirmations. Only
the independently validated foreground Main/Pause top scene can resume input;
observed ineligible scenes/category changes, invalid viewports or a changed
provider session discard stale UI. No Stop, Apply, retry or command replay is
generated by focus changes. An already submitted command remains provider-owned.

### Limits and privacy

- 16 providers; 8 pages and 64 total controls per provider; one image.
- Declaration 64 KiB; state 256 KiB; depth 16; commands 32 KiB including form and NUL.
- 16 queued/in-flight requests; full queues refuse without overwriting work.
- 128 rows/list, 256 rows/provider; stable IDs 1..64 bytes.
- Labels 160 bytes; status/reason/help 1024 bytes; text value up to 4096 bytes.
- Pixel image 1..64 by 1..48, exactly width×height uint32 values. Display at
  most 4 updates/second; low-resolution visualization, not texture/3D rendering.

Never send passwords, access/refresh tokens, cookies or pack keys through the
interface or ordinary JSON config. ModMenu does not log or persist workspace
payloads, but cannot sanitize secrets deliberately sent by a native provider.
The provider owns secure credentials and official explicit authentication.

Existing BedrockTool builds pin a previous ModMenu hash and are not changed by this
update. Its adapter must separately review and pin the new binary before joint
testing. Do not bypass that guard or infer native-hook compatibility from the
generic API tests.

## 繁體中文

**0.6.0-test3 是選用的實驗擴充。** v1／v2 不變；v3 只新增通用文字設定，
工作頁使用獨立的 v1 協定，不需要 LeviLamina，也不寫死 BedrockTool 功能。

文字設定宣告 `minBytes`／`maxBytes`（UTF-8 位元組，最多 4096），選用
`validation: none` 或 `identifier`。地址、正規表示式、排除清單及路徑均為
單行字串，語意由插件驗證；編輯不會連網、讀取路徑或執行腳本。
支援方向鍵、Home／End、Shift 選取、Ctrl+A／C／V、刪字及 Unicode 提交文字；
不提供原生輸入法組字視窗。Enter 完成編輯、Escape 取消目前編輯；Apply／Cancel
沿用交易式儲存，儲存成功不等於插件已套用。沒有密碼／權杖欄位。
空白、無法讀取或超限的剪貼簿會保留原有選取文字並顯示錯誤，不偷偷截斷貼上內容。

插件在自己的工作執行緒，以 [SDK](sdk/modmenu_workspace_v1.h) 的四個 V1 介面
註冊頁面、發布完整快照、取出命令及取消註冊。資料在呼叫期間複製，不傳 Go 指標、
C++ 物件或回呼。資料夾、DLL 與 manifest 精確版本必須符合；只接受已載入且與
ModMenu 位於相鄰插件目錄的 DLL。註冊會固定 DLL 至程序結束，不提供熱卸載。
這是同程序原生插件的合作介面，不是隔離惡意 DLL 的安全沙箱。

控件包含 `text`、`boolean`、`action`、`status`、`progress`、`list`、`image`。
插件自行命名頁面／命令，提供英文及選用的繁簡中文；沿用遊戲語言及返回方式。
Configure 被動設定與 Workspace 工作草稿分開。只有明確點擊或鍵盤啟用才送命令，
危險動作可要求確認；開頁、切頁、繪製及更新狀態不操作。關頁不送 Stop、不取消工作。

命令含工作階段、請求編號、狀態版本、頁面、動作及該頁表單；選取清單另帶穩定
`rowId`。插件必須再次驗證參數與當前狀態，以 `succeeded`／`failed` 回報結果。
送達不等於完成；同一動作在回覆前不可重送，不自動重試。短 Poll 緩衝不取走命令。

限制為每插件 8 頁／64 控件、描述 64 KiB、狀態 256 KiB、命令含 NUL 共 32 KiB、
16 筆等候／執行請求、每清單 128 列／總共 256 列、最多一張 64×48 RGBA8 圖像。
圖像直接交付有界像素，沒有 URL／圖片路徑／材質控制代碼；建議每秒最多更新 4 次。
超限、未知屬性、重複 JSON 鍵及無效資料整筆拒絕，保留上一筆有效快照。

帳密、權杖、Cookie 及資源包金鑰不可經一般設定、命令或快照傳送。ModMenu 不記錄
或持久化工作頁內容，但無法代替插件保護其自行傳入的秘密。登入與憑證存放由插件負責。

暫時失焦會保留原分頁、捲動及已確認表單值，只暫停輸入並取消按下配對、綁鍵、
未完成編輯／貼上及危險確認。只有重新核實的前景主選單／暫停選單可以恢復操作；
觀察到不允許畫面、場景類別變化、無效視窗或提供者 session 變更時會丟棄舊頁面。
不因失焦自動 Apply／Stop／重試／重送，已提交工作仍由提供者處理。

現有 BedrockTool 固定舊 ModMenu 雜湊，本輪不修改它；聯合測試前，仍需由其適配層
獨立審核並固定新二進位，不可移除檢查繞過，也不能以公用 API 測試推定原生相容性。
可先用獨立示範 DLL 驗收。
