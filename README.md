# ModMenuBedrock

[English](#english) · [繁體中文](#繁體中文)

## English

A native plugin menu for Minecraft Bedrock and LeviLauncher, inspired by
[Java Mod Menu](https://github.com/TerraformersMC/ModMenu).
No LeviLamina API, resource pack or server component is required.

**Windows x64 · Minecraft 1.26.45.01 / 1.26.51.01 / 1.26.52.03**

**0.6.0-test5-leftalt — Prerelease · MIT License**

[Download](https://github.com/XuanDiNa/ModMenuBedrock/releases/tag/v0.6.0-test5-leftalt) ·
[Report an issue](https://github.com/XuanDiNa/ModMenuBedrock/issues)

### Features

- Mods buttons in the supported vanilla main and pause menus.
- F10 by default, with a customizable shortcut or no binding.
- Searchable plugin list, details and Configure pages; only loaded plugins are listed.
- Shared settings controls, categories, help, per-field reset and Apply/Cancel.
- Optional sliders for signed integers and fixed-precision decimals, with dragging,
  direct input and keyboard adjustment.
- English, Traditional Chinese and Simplified Chinese, following the game language.
- Ctrl / Shift / Left Alt combinations for compatible plugins; Esc clears a binding.
- Optional UTF-8 text fields and asynchronous plugin workspaces.
- Navigation and committed drafts survive temporary focus loss; transient input is cancelled.
- Exact-version safety checks and compatibility handling for supported shared hooks.

### Installation

1. Close Minecraft and back up the existing `mods/ModMenuBedrock` folder.
2. Extract the release ZIP and copy its `ModMenuBedrock` folder into the selected
   game version's `mods` directory. Keep your existing `config.json`, logs and
   launcher enabled/disabled state.
3. Start through LeviLauncher. Click Mods in the main or pause menu, or press F10.

One menu DLL supports the three exact builds above. Choose the menu-only
`ModMenuBedrock-0.6.0-test5-leftalt-win-x64.zip`, or the **1.26.52.03-only**
`ModMenu-matched-bindings-26.52-win-x64.zip`. The latter contains matching
AutoClick, AutoJumpReset, ContainerPreview, FakeSneak, FullBright, ToggleSprint
and MuoniumPlayerBedrock updates. Merge its `mods` directory after backing up
affected folders **outside** the active mods directory. Replace each selected
DLL, manifest and description together; keep all settings and unrelated files.
Neither update includes user settings. Do not install integration examples or
load duplicate DLLs.

Muonium is an update-only DLL for an existing **0.7.0-test1** installation;
keep its Java files, runtime and player data. The bundle's read-only
`CHECK-INSTALL.ps1` checks all eight complete matching sets.
Keep only the manifest name matching the launcher state (`manifest.json` or
`manifest.json.close`), not both.

### Configuration

Configure edits remain drafts until Apply; Cancel discards them. While binding a
key, Escape clears the binding when the field allows it. Outside key capture,
Escape goes back one page; leaving Mods opened from pause keeps the game paused.
F10 does not open Mods from the HUD, inventory or vanilla settings.

For a combination, click the binding field, hold Ctrl/Shift/Left Alt, then press
the main key. Standalone modifiers are captured on release. Esc selects Unbound;
Apply saves it. Mouse buttons cannot be bound, and Windows-reserved shortcuts
may be consumed by the operating system. See [key bindings](integration/KEYBINDINGS.md)
and [workspaces](integration/WORKSPACES.md) for the public SDK.

Built-in adapters cover MouseTweaksBedrock, AutoRestock, ContainerPreview,
FakeSneak, AutoClick and AutoJumpReset. Other authors can add a passive
`modmenu.settings.json`: see the [integration guide](integration/README.md) and
[slider guide](integration/SLIDERS.md). Sliders are opt-in; existing controls
are not converted automatically. Plugins remain responsible for hot reloading
their saved settings.

### Compatibility and feedback

This release passed offline build, input, settings and package checks; in-game
acceptance of these new DLLs is still pending. The earlier
[0.5.0-test1](https://github.com/XuanDiNa/ModMenuBedrock/releases/tag/v0.5.0-test1)
was manually tested on all three builds; that result does not validate this release.
Long sessions, every GUI scale, OreUI and arbitrary
resource-pack menu replacements have not been exhaustively verified. Unknown
game images or incompatible shared-hook owners leave ModMenu inactive; update
an old Mouse Tweaks build rather than bypassing those checks.

**BedrockToolNative 0.2.0-test5 pins the older ModMenu test3 hash and will reject
this DLL.** Its Workspace requires a separate companion update; do not bypass
the check. The seven-plugin bundle does not add game-version support.
See [release notes](docs/releases/0.6.0-test5-leftalt.md) and
[binding tests and rollback](docs/keybindings-testing.md).

For issues, include the exact game/plugin versions, operation and relevant log
excerpt, with personal information removed. See [testing](TESTING.md) and
[release verification](docs/verification.md) for scope and artifact provenance.

Source is available under [MIT](LICENSE); dependency notices are in
[THIRD_PARTY_NOTICES.txt](THIRD_PARTY_NOTICES.txt).
Build instructions: [BUILDING.md](BUILDING.md).

---

## 繁體中文

參考 [Java Mod Menu](https://github.com/TerraformersMC/ModMenu) 製作的
Minecraft 基岩版／LeviLauncher 原生插件選單。
不需要 LeviLamina API、資源包或服務端組件。

**Windows x64 · Minecraft 1.26.45.01 / 1.26.51.01 / 1.26.52.03**

**0.6.0-test5-leftalt — 預發佈版 · MIT 授權**

[下載](https://github.com/XuanDiNa/ModMenuBedrock/releases/tag/v0.6.0-test5-leftalt) ·
[回報問題](https://github.com/XuanDiNa/ModMenuBedrock/issues)

### 功能

- 在受支援的原版主選單與暫停選單加入 Mods 按鈕。
- 預設 F10，可自訂快捷鍵或設為無綁定。
- 插件搜尋、詳細資料與 Configure 設定頁，只列出實際載入的插件。
- 共用設定控制項、分類、說明、單項重設及 Apply／Cancel。
- 選用滑動條支援正負整數與固定精度小數，可拖曳、直接輸入及鍵盤微調。
- 跟隨遊戲語言，提供英文、繁體中文與簡體中文。
- 相容插件支援 Ctrl／Shift／左 Alt 組合鍵，綁定時按 Esc 解除綁定。
- 選用 UTF-8 文字欄位與非同步插件工作頁。
- 暫時失焦時保留導覽及已提交草稿值，取消尚未完成的輸入操作。
- 精確遊戲版本檢查，以及受支援的共用 Hook 相容處理。

### 安裝

1. 完全關閉 Minecraft，備份現有 `mods/ModMenuBedrock` 資料夾。
2. 解壓 Release ZIP，將其中的 `ModMenuBedrock` 放入所選遊戲版本的 `mods`
   目錄。保留原有 `config.json`、日誌及啟動器啟用／停用狀態。
3. 透過 LeviLauncher 啟動，點擊主選單／暫停選單的 Mods，或按 F10。

上述三個精確版本共用一份 ModMenu DLL。可選單獨包
`ModMenuBedrock-0.6.0-test5-leftalt-win-x64.zip`，或僅適用 **1.26.52.03** 的
`ModMenu-matched-bindings-26.52-win-x64.zip`；後者包含 AutoClick、AutoJumpReset、
ContainerPreview、FakeSneak、FullBright、ToggleSprint 及 MuoniumPlayerBedrock 配套版。
先把受影響插件備份到 **mods 以外**，再合併包內 `mods`，一起替換所選插件的 DLL、
manifest 與描述檔，保留原有設定及其他檔案。兩種更新均不含使用者設定檔；
不要把接入範例當成插件安裝，也不要同時載入新舊 DLL。

Muonium 是既有 **0.7.0-test1** 的 DLL 更新，並非完整播放器；請保留 Java、runtime
及播放器資料。配套包的唯讀 `CHECK-INSTALL.ps1` 可核對八組完整檔案是否一致。
依啟用狀態只保留 `manifest.json` 或 `manifest.json.close`，不要同時保留兩者。

### 設定

Configure 在 Apply 前只修改草稿，Cancel 不儲存。等待綁鍵時，若該欄位允許，
Escape 會解除綁定；其他情況下 Escape 逐層返回。從暫停選單開啟 Mods 後退出，
仍停留在暫停選單。F10 不會在 HUD、背包或原版設定頁開啟 Mods。

點擊綁鍵欄位，先按住 Ctrl／Shift／左 Alt，再按主鍵；單獨綁修飾鍵時放開確認。
Esc 設為無綁定，Apply 才儲存。滑鼠鍵不可綁，Windows 保留組合可能被系統攔截。
公用 SDK 見[快捷鍵](integration/KEYBINDINGS.md)及[工作頁](integration/WORKSPACES.md)。

內建適配器提供 MouseTweaksBedrock、AutoRestock、ContainerPreview、FakeSneak、
AutoClick 及 AutoJumpReset 的設定頁。其他作者可隨附被動的
`modmenu.settings.json`，詳見[接入指南](integration/README.md)及
[滑動條指南](integration/SLIDERS.md)。滑動條需要明確接入，不會自動轉換既有控件；
設定儲存後是否熱更新，仍由各插件負責。

### 相容性與回饋

本版已通過離線建置、輸入、設定及套件檢查，新 DLL 的遊戲內驗收仍待完成。
先前 [0.5.0-test1](https://github.com/XuanDiNa/ModMenuBedrock/releases/tag/v0.5.0-test1)
的三版本人工測試不代表本版已驗收。長時間使用、所有 GUI
比例、OreUI 及任意資源包改版選單仍未全面驗證。未知遊戲映像或不相容的共用 Hook
會讓 ModMenu 保持未啟用；若使用舊版 Mouse Tweaks，請更新它，不要繞過檢查。

**BedrockToolNative 0.2.0-test5 綁定舊 ModMenu test3 雜湊，會拒絕此 DLL。**
其工作頁需要另外配套更新，請勿繞過檢查。七插件包不擴充各插件的版本範圍。
詳見[發佈說明](docs/releases/0.6.0-test5-leftalt.md)及[快捷鍵測試與回退](docs/keybindings-testing.md)。

回報問題時，請提供精確遊戲／插件版本、操作與相關日誌片段，並移除個人資訊。
測試範圍及檔案來源見[測試說明](TESTING.md)與[發佈驗證](docs/verification.md)。

原始碼採用 [MIT 授權](LICENSE)，依賴聲明見
[THIRD_PARTY_NOTICES.txt](THIRD_PARTY_NOTICES.txt)；編譯方式見
[BUILDING.md](BUILDING.md)。
