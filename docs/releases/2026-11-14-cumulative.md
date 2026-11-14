# Changes since 0.6.0-test5-leftalt / 上次公開版之後的更新

## English

The previous public release was **0.6.0-test5-leftalt**. This release publishes
the independent **ModMenuBedrockLL 0.1.0-test2-ll26.60** edition. It includes the
26.60 menu adapter and resource-pack entry work developed in the intervening
local versions. The vanilla distribution is in `editions/vanilla` on `main`; this LL
package supports only **Minecraft 1.26.60.05 / LeviLamina 26.60.7 / Windows x64**.

| Local version | Changes and status |
| --- | --- |
| 0.6.0-test6-layer-diag1 | Added bounded diagnostics for layered menu resources to investigate a missing Mods entry. It was a diagnostic build, not a confirmed console-related fix. No resource payload or credential logging was added. |
| 0.6.0-test7-resourcepack | Added the public JSON UI event `button.modmenu.open`, retained the legacy event, and supplied entry ownership rules, an integration guide and an optional example. Pack authors can draw and position their own Mods entry and choose its label. Arbitrary menu packs are not modified automatically. |
| 0.6.0-test8-mc26.60 | Added an independently reviewed 1.26.60.05 native profile and adapted scene, drawing and input layouts. Automatic insertion on 26.60 admits only reviewed single-layer vanilla definitions; modified or layered menus must integrate the public entry. The vanilla candidate retains its earlier game profiles, while this LL edition deliberately targets only 26.60. |
| 0.1.0-test1-ll26.60 | Created the separate LL native client edition using the official mod registration, load/enable/disable lifecycle, build link rule and shared allocator. Added discovery for loaded native manifests and an LL Workspace example. Its own DLL, folder, settings and log are distinct from the vanilla edition. |
| 0.1.0-test2-ll26.60 | Added the exact GUI/console image pair produced by installing PreLoader before LeviLauncher. The console reconstruction matches the player's complete B554A2DA… SHA-256; the old test1 refused it at `disk-identity`. Whole-file, code, loaded-header, entry, hook-owner, scene and foreground guards remain. |

Settings descriptions v1-v5, binding tokens, Workspace V1 and resource-pack
entry API v1 retain their contracts. Sliders, shared text fields, asynchronous
workspaces and Ctrl/Shift/Left Alt chords were already part of the previous
public release; they are retained here. Providers must resolve the LL DLL and
review any binary pins themselves. Updating ModMenu does not update another
plugin's runtime or hot-reload logic.

**Choose the right archive.** Existing LL installations should use the `update`
ZIP, which excludes `config.json`; new installations can use the full
`windows-x64` ZIP. The source and integration SDK ZIPs are for developers.
Completely exit Minecraft before updating. Do not load the vanilla and LL DLLs
together. Keep the existing game EXE, console option and unrelated mods unchanged
for the affected-player check.

**Validation:** Debug and Release each pass 28 offline suites, including 57
native-image and 20 lifecycle checks. Settings/schema audits and 94 resource-pack
entry checks pass. The original eight game sections, entrypoint and all 1,012
original imported symbols across 88 DLL descriptors were audited in isolated
fixtures. Unknown images remain refused. No game file, private fixture, account
data, IDA database or other plugin DLL is distributed.

The affected player's test2 gameplay acceptance and the combined music-plugin
test remain pending. Menu appearance/merging in arbitrary packs, OreUI and all
third-party Workspace providers are not established by these offline checks.
The historic console-dependence report remains unproven as a console cause.

See [LL test2 details](https://github.com/XuanDiNa/ModMenuBedrock/blob/codex/levilamina-edition/docs/releases/0.1.0-test2-ll26.60.md), [testing](https://github.com/XuanDiNa/ModMenuBedrock/blob/codex/levilamina-edition/TESTING.md),
[resource-pack integration](https://github.com/XuanDiNa/ModMenuBedrock/blob/codex/levilamina-edition/integration/RESOURCE_PACKS.md) and
[provider migration](https://github.com/XuanDiNa/ModMenuBedrock/blob/codex/levilamina-edition/docs/levilamina-provider-migration.md).

## 繁體中文

上一個 GitHub 公開版是 **0.6.0-test5-leftalt**。本次公開獨立的
**ModMenuBedrockLL 0.1.0-test2-ll26.60**，包含這段期間本機版本新增的
26.60 選單適配與材質包入口。原版路線位於 `main` 的 `editions/vanilla`；本 LL 套件只支援
**Minecraft 1.26.60.05／LeviLamina 26.60.7／Windows x64**。

| 本機版本 | 新增內容與狀態 |
| --- | --- |
| 0.6.0-test6-layer-diag1 | 新增有界的選單資源分層診斷，用來調查 Mods 入口缺失。這是診斷版，不能宣稱已修復「不開控制台就不能用」；不記錄資源內容或憑證。 |
| 0.6.0-test7-resourcepack | 新增公用 JSON UI 事件 `button.modmenu.open`，保留舊事件，提供入口接管規則、接入文件與選用範例。材質包作者可自行繪製 Mods 入口、安排位置及文字；不會自動修改所有選單材質包。 |
| 0.6.0-test8-mc26.60 | 新增獨立核實的 1.26.60.05 原生配置，適配場景、繪製與輸入結構。26.60 自動入口只接受已核實的單層原版選單；修改或多層選單需主動接入公用入口。原版候選保留舊三版本；本 LL 版則只啟用 26.60。 |
| 0.1.0-test1-ll26.60 | 製作獨立 LL native 客戶端版，使用官方註冊、載入／啟用／停用生命週期、建置連結規則與共用配置器。支援已載入 native manifest 的插件發現及 LL Workspace 範例；DLL、資料夾、設定與日誌皆獨立。 |
| 0.1.0-test2-ll26.60 | 補齊「先 PreLoader、後 LeviLauncher」安裝順序產生的精確 GUI／控制台映像。完整重建與玩家 B554A2DA… 雜湊相同，解決 test1 在 `disk-identity` 拒絕此映像的問題。保留完整檔案、程式碼、載入標頭、入口、掛鉤擁有者、場景及前景檢查。 |

設定描述檔 v1-v5、綁鍵代碼、Workspace V1 與材質包入口 v1 的公用規格保持相容。
滑動條、共用文字欄位、非同步工作頁及 Ctrl／Shift／左 Alt 組合鍵已在上次公開版
提供，本版沿用。提供者仍需明確接入 LL DLL，自行審核限定的二進位版本；更新
ModMenu 不會同步更新其他插件的執行或設定熱更新邏輯。

**套件選擇：** 已安裝 LL 版時使用 `update` ZIP，不包含 `config.json`；首次安裝
使用 `windows-x64` 完整 ZIP。原始碼與整合 SDK 供開發者使用。完全退出 Minecraft
後再更新，不可同時載入原版及 LL 版。原玩家驗收時請保留原 EXE、控制台選項及
其他插件，避免同時改變測試條件。

**驗證範圍：** Debug／Release 每種模式通過 28 組離線測試，包含 57 項原生映像
與 20 項生命週期檢查；設定／Schema 與 94 項材質包入口檢查通過。隔離資料另核實
原遊戲 8 個區段、入口及 88 個 DLL 描述中的 1,012 個原始匯入函式。未知映像仍被
拒絕。套件不含遊戲檔、私有資料樣本、帳號、IDA 資料庫或其他插件 DLL。

原玩家的 test2 實機驗收及與音樂插件的共同測試仍待完成。離線測試不能代替任意
材質包的畫面／合併驗收、OreUI 或所有第三方 Workspace 提供者的相容性測試。
過去控制台相關回報的因果關係仍未證實。

詳見上方的本版詳情、測試、材質包接入與提供者遷移文件。
