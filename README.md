# ModMenuBedrock

[English](#english) · [繁體中文](#繁體中文)

## English

A native plugin menu for Minecraft Bedrock and LeviLauncher, inspired by
[Java Mod Menu](https://github.com/TerraformersMC/ModMenu).
No LeviLamina API, resource pack or server component is required.

**Windows x64 · Minecraft 1.26.45.01 / 1.26.51.01 / 1.26.52.03**

**0.5.0-test1 — Prerelease · MIT License**

[Download](https://github.com/XuanDiNa/ModMenuBedrock/releases/tag/v0.5.0-test1) ·
[Report an issue](https://github.com/XuanDiNa/ModMenuBedrock/issues)

### Features

- Mods buttons in the supported vanilla main and pause menus.
- F10 by default, with a customizable shortcut or no binding.
- Searchable plugin list, details and Configure pages; only loaded plugins are listed.
- Shared settings controls, categories, help, per-field reset and Apply/Cancel.
- Optional sliders for signed integers and fixed-precision decimals, with dragging,
  direct input and keyboard adjustment.
- English, Traditional Chinese and Simplified Chinese, following the game language.
- Exact-version safety checks and compatibility handling for supported shared hooks.

### Installation

1. Close Minecraft and back up the existing `mods/ModMenuBedrock` folder.
2. Extract the release ZIP and copy its `ModMenuBedrock` folder into the selected
   game version's `mods` directory. Keep your existing `config.json`, logs and
   launcher enabled/disabled state.
3. Start through LeviLauncher. Click Mods in the main or pause menu, or press F10.

One DLL supports the three exact builds above. Do not install the integration
examples as plugins or overwrite other plugins' files.
Keep only the manifest name matching the launcher state (`manifest.json` or
`manifest.json.close`), not both.

### Configuration

Configure edits remain drafts until Apply; Cancel discards them. While binding a
key, Escape clears the binding when the field allows it. Outside key capture,
Escape goes back one page; leaving Mods opened from pause keeps the game paused.
F10 does not open Mods from the HUD, inventory or vanilla settings.

Built-in adapters cover MouseTweaksBedrock, AutoRestock, ContainerPreview,
FakeSneak, AutoClick and AutoJumpReset. Other authors can add a passive
`modmenu.settings.json`: see the [integration guide](integration/README.md) and
[slider guide](integration/SLIDERS.md). Sliders are opt-in; existing controls
are not converted automatically. Plugins remain responsible for hot reloading
their saved settings.

### Compatibility and feedback

Basic in-game use of this exact DLL has been tested by the maintainer on all
three supported versions. Long sessions, every GUI scale, OreUI and arbitrary
resource-pack menu replacements have not been exhaustively verified. Unknown
game images or incompatible shared-hook owners leave ModMenu inactive; update
an old Mouse Tweaks build rather than bypassing those checks.

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

**0.5.0-test1 — 預發佈版 · MIT 授權**

[下載](https://github.com/XuanDiNa/ModMenuBedrock/releases/tag/v0.5.0-test1) ·
[回報問題](https://github.com/XuanDiNa/ModMenuBedrock/issues)

### 功能

- 在受支援的原版主選單與暫停選單加入 Mods 按鈕。
- 預設 F10，可自訂快捷鍵或設為無綁定。
- 插件搜尋、詳細資料與 Configure 設定頁，只列出實際載入的插件。
- 共用設定控制項、分類、說明、單項重設及 Apply／Cancel。
- 選用滑動條支援正負整數與固定精度小數，可拖曳、直接輸入及鍵盤微調。
- 跟隨遊戲語言，提供英文、繁體中文與簡體中文。
- 精確遊戲版本檢查，以及受支援的共用 Hook 相容處理。

### 安裝

1. 完全關閉 Minecraft，備份現有 `mods/ModMenuBedrock` 資料夾。
2. 解壓 Release ZIP，將其中的 `ModMenuBedrock` 放入所選遊戲版本的 `mods`
   目錄。保留原有 `config.json`、日誌及啟動器啟用／停用狀態。
3. 透過 LeviLauncher 啟動，點擊主選單／暫停選單的 Mods，或按 F10。

上述三個精確版本共用一份 DLL。不要把接入範例當成插件安裝，也不要覆蓋其他插件檔案。
依啟用狀態只保留 `manifest.json` 或 `manifest.json.close`，不要同時保留兩者。

### 設定

Configure 在 Apply 前只修改草稿，Cancel 不儲存。等待綁鍵時，若該欄位允許，
Escape 會解除綁定；其他情況下 Escape 逐層返回。從暫停選單開啟 Mods 後退出，
仍停留在暫停選單。F10 不會在 HUD、背包或原版設定頁開啟 Mods。

內建適配器提供 MouseTweaksBedrock、AutoRestock、ContainerPreview、FakeSneak、
AutoClick 及 AutoJumpReset 的設定頁。其他作者可隨附被動的
`modmenu.settings.json`，詳見[接入指南](integration/README.md)及
[滑動條指南](integration/SLIDERS.md)。滑動條需要明確接入，不會自動轉換既有控件；
設定儲存後是否熱更新，仍由各插件負責。

### 相容性與回饋

維護者已在三個支援版本人工測試此份 DLL，基本使用正常。長時間使用、所有 GUI
比例、OreUI 及任意資源包改版選單仍未全面驗證。未知遊戲映像或不相容的共用 Hook
會讓 ModMenu 保持未啟用；若使用舊版 Mouse Tweaks，請更新它，不要繞過檢查。

回報問題時，請提供精確遊戲／插件版本、操作與相關日誌片段，並移除個人資訊。
測試範圍及檔案來源見[測試說明](TESTING.md)與[發佈驗證](docs/verification.md)。

原始碼採用 [MIT 授權](LICENSE)，依賴聲明見
[THIRD_PARTY_NOTICES.txt](THIRD_PARTY_NOTICES.txt)；編譯方式見
[BUILDING.md](BUILDING.md)。
