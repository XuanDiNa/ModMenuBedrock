# Plugin integration / 插件接入

**Resource-pack authors / 材質包作者:** optional
[JSON UI entry API v1 / JSON UI 按鈕入口介面](RESOURCE_PACKS.md), introduced in
`0.6.0-test7-resourcepack`, lets a pack render its own Mods button. Existing
settings and workspace contracts below stay unchanged.

**0.6.0-test5-leftalt:** optional [keyboard bindings v5 / 公用鍵盤綁定](KEYBINDINGS.md)
adds broad keyboard coverage and exact Ctrl/Shift/Left Alt combinations. Existing v1/v2/v3/v4
contracts and the Workspace ABI remain unchanged.

**0.6.0-test3:** optional public text settings v3 and generic asynchronous
[Workspaces / 公用工作頁](WORKSPACES.md). The v1/v2 contracts below are unchanged.
Temporary focus loss suspends input without closing valid navigation; scene and
provider-session invalidation remain enforced. No provider API update is required.

暫時失焦只暫停輸入並保留有效頁面；場景及提供者工作階段失效保護不變，無須更新 API。

**0.5.0-test1 adds optional v2 sliders.** Existing v1 behavior is unchanged.
See [Sliders / 滑動條](SLIDERS.md) and the separate v2 schema/examples.
**0.5.0-test1 新增選用的 v2 滑動條。** 以下 v1 規格保持不變；新控件見上方連結。

## English

**Passive settings description v1, introduced in ModMenuBedrock 0.4.0-test1.**
No LeviLamina, callable DLL API, scripts, packets or native offsets are needed
for settings integration. The menu must independently support the game version.

Ship these files inside the plugin's own existing loader folder:

```text
mods/ExamplePlugin/
  ExamplePlugin.dll          # your actual plugin; not provided by this template
  manifest.json             # loader entry and exact plugin version
  settings.json             # existing config understood by your plugin
  modmenu.settings.json     # shared UI description
```

`examples/ExamplePlugin` is an author template, **not a runnable mod**. It includes
all four controls. Do not install it expecting a list entry without a loaded DLL.
The examples include the generic four-control template and focused real-plugin
descriptions for AutoRestock and FastBlockBreak. Offline integration checks use
temporary directories and do not load fixture DLLs. No installation is automatic.

### Matching and fallback

- The catalog remains loaded-only; a description never loads/unloads a DLL.
- `plugin.id` equals the folder name (ASCII letters/digits/underscore/hyphen).
  `entry` exactly matches the manifest DLL basename. `versions` contains exact
  manifest `version` strings, not mod.json's display version. No wildcards/ranges.
- The sidecar has one fixed name, `modmenu.settings.json`, and `schemaVersion: 1`.
  When absent, the six legacy built-in adapters remain available. When present
  but invalid/incompatible, Configure is refused instead of falling back.
- Availability refreshes about every 2 seconds. A description can be added or
  removed while the menu is closed. Cancel and reopen Configure after any edit.
  Changing a loaded DLL/launcher enable state still requires a game restart.
- Matching a manifest is not a signature/hash check of the plugin executable.
  Install descriptions only from a trusted plugin author; a wrong declared
  schema can change that plugin's configuration incorrectly.

### Controls and text

Each field has `path`, `type`, `default`, `label`; optionally `category` and
`description`. English text is a plain string, or a locale object with required
`en_US` and optional `zh_TW` / `zh_CN`. Missing translation falls back to the
author's English, not Windows language. Reuse the exact same category translation
for all fields in one category; `All` is reserved. Labels/categories/options are
single-line; descriptions may contain line breaks. No Minecraft formatting codes.

| Type | Additional fields | Saved value |
| --- | --- | --- |
| `boolean` | none | Boolean |
| `integer` | `min`, `max`; optional positive `step`, short `unit` | Integer in 0..2147483647 and declared bounds |
| `choice` | 2..6 `options` captions | Zero-based integer index |
| `key` | `keys` supported-token allowlist | One canonical token, e.g. `F6` or `LALT` |

No string/decimal/negative-number editor, slider, chord or arbitrary action.
The test3 built-in AutoClick/AutoJumpReset adapters have private validated text
and native-VK formats; these are not new field types in the external v1 contract.
Key tokens are uppercase. Since test4, Escape clears to `NONE` when it is in the
field's allowlist; otherwise it cancels capture without changing the value.
Backspace also maps to `NONE` only if permitted. Authors must really implement
NONE as no binding; do not advertise it if the runtime keeps an old/default key.
Copy the template and adjust it to the plugin's
real parser: names/defaults/ranges are not inferred. Reset changes the draft only.

`path` is an object-only RFC6901 JSON pointer. `~0` and `~1` escape `~` and `/`.
No empty/numeric segments, arrays, root replacement, duplicates or parent/child
overlap; at most 16 segments/256 UTF-8 bytes. Existing parents must be objects.
Config members not exposed by the description are preserved.

### Saving, reload and validation

`config.file` is one ordinary ASCII `.json` basename in the plugin folder, not a
path/device/stream/manifest/metadata/description. Existing target required; neither
Configure nor discovery creates a missing config. Config/description size limit
is 64 KiB, fields 1..64, exact versions 1..32. Labels/options 160 UTF-8 bytes,
categories 80, descriptions 1024, units 16. JSON depth at most 32, no duplicate
members, unknown description properties or malformed UTF-8. Links/reparse points
are refused. Read-only/locked configs remain untouched.

`reload: hot` declares that the plugin itself watches/reloads the config;
`reload: restart` tells the user to restart Minecraft. The menu writes JSON only:
it does not call plugin code or receive an acknowledgement. Apply makes an exact
original backup and atomically replaces the config, preserving unknown data.
Cancel never saves. Manifest/description/config changes while editing refuse
stale Apply; authoritative source handles are held read-only during saving.
Concurrent hostile directory manipulation is not a supported editing workflow.

The included JSON Schema is editor assistance, not the runtime authority. It
cannot enforce UTF-8 byte limits, filesystem identity, exact installed-version
matching, all pointer/overlap rules or cross-field default/range/key consistency.
The runtime parser enforces those checks; `$schema` is never fetched from network.

## 繁體中文

**ModMenuBedrock 0.4.0-test1 起提供被動 JSON 設定描述規格 v1。**
設定接入不需要 LeviLamina、DLL 回呼、腳本或原生位址；遊戲版本支援仍由 ModMenu
本身決定。插件繼續放在自己的載入目錄，隨附現有 DLL、manifest、設定檔及固定名稱
`modmenu.settings.json`。列表仍只顯示已載入 DLL，描述檔不負責載入／卸載。

`ExamplePlugin` 是四種控件的作者範本，不含 DLL，不是可執行插件。範例另包含
AutoRestock 與 FastBlockBreak 的精簡實際插件描述檔。離線測試使用臨時目錄，不載入
範例 DLL，也不會自動安裝插件。

- `plugin.id` 必須等於 ASCII 資料夾名稱，`entry` 與 manifest DLL 檔名完全一致；
  `versions` 列出精確的 manifest 版本，不使用 mod.json 顯示版本、萬用字元或範圍。
- 沒有描述檔時沿用六個舊適配器；存在但無效／不相容時，只拒絕該插件 Configure，
  不回退另一套欄位、不隱藏已載入插件。約 2 秒更新；修改後取消並重開 Configure。
- 此匹配不是 DLL 簽章／雜湊驗證。只使用可信作者的描述檔；作者提供錯誤欄位仍可能
  改錯該插件設定。更換 DLL 或載入啟用狀態依然需要重啟遊戲。

每個欄位需要 `path`、`type`、`default`、`label`；可選 `category`、`description`。
文字可為英文，或含必填 `en_US`、選填 `zh_TW`／`zh_CN` 的物件；缺譯回退作者英文。
同分類必須使用一致翻譯，`All` 為保留分類。標籤／分類／選項為單行，說明允許換行，
不允許 Minecraft 格式代碼。翻譯不改變路徑、選項索引或按鍵代碼。

四種控件：`boolean` 開關；`integer` 非負整數，需要 min/max，可設正數 step/unit；
`choice` 2–6 個選項，儲存從 0 起算的整數索引；`key` 使用 keys 白名單，儲存單一
大寫按鍵代碼。沒有文字／小數／負數編輯、滑桿、組合鍵或任意動作。test4 起 Escape
在 keys 允許 NONE 時解除綁定，否則只取消綁鍵；Backspace 也只在允許時綁 NONE。
插件必須真的實作無綁定，不可宣告 NONE 卻沿用舊鍵／預設鍵。預設、範圍與欄位必須
對應真實解析器，不自動推測。
test3 的 AutoClick／AutoJumpReset 內建適配器有私有驗證文字與原生 VK 格式，
不代表外部 v1 描述新增了文字欄位或任意按鍵格式。

path 為僅物件的 RFC6901 指標，`~0`／`~1` 表示 `~`／`/`；不能使用空／純數字段、
陣列、根替換、重複或父子重疊，最多 16 段／256 UTF-8 位元組。既有父層必須為物件。
設定檔只允許插件資料夾內普通 ASCII `.json` 檔名，不接受路徑、裝置、資料流或保留
中繼檔；目標必須已存在，不自動建立。描述檔／設定檔各限 64 KiB、欄位 1–64、版本
1–32；標籤／選項 160、分類 80、說明 1024、單位 16 UTF-8 位元組；巢狀最多 32。
重複鍵、未知描述屬性、無效 UTF-8、連結／重新解析點均拒絕，鎖定／唯讀設定不改動。

`reload: hot` 只是作者宣告插件自行重讀設定，`restart` 則提示重啟 Minecraft；
選單只寫 JSON，不呼叫插件或收到套用確認。Apply 保留精確備份、原子替換及未公開
欄位；Cancel／重設草稿不寫檔。編輯期間來源改變會拒絕儲存，交易期間鎖住權威來源；
不支援惡意並行修改目錄的工作流程。

隨附 JSON Schema 僅輔助編輯器；位元組上限、檔案身份、已安裝精確版本、全部指標／
重疊及預設／範圍等交叉限制由執行期解析器檢查，`$schema` 不會透過網路下載。
