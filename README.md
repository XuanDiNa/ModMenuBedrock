# ModMenuBedrockLL

A native ModMenu client for **Minecraft Bedrock 1.26.60.05 / LeviLamina 26.60.7 / Windows x64**. Independent test release: **0.1.0-test2-ll26.60**.

[Download](https://github.com/XuanDiNa/ModMenuBedrock/releases/tag/v0.1.0-test2-ll26.60) · [Changes since the previous public release](docs/releases/2026-11-14-cumulative.md) · [Vanilla edition](https://github.com/XuanDiNa/ModMenuBedrock/tree/main)

Use the `update` archive to retain an existing LL `config.json`; use the full `windows-x64` archive for a new installation. The release contains this independent edition only.

Browse loaded mods from the main or pause menu, edit supported settings, and open opt-in plugin workspaces. Includes game-language localization, key/chord capture, sliders, text fields, and resource-pack entry API `button.modmenu.open`.

With Minecraft closed, extract the package to the instance root, creating `mods/ModMenuBedrockLL/`. Disable the vanilla ModMenuBedrock edition before starting. Do not rename the DLL. The manifest uses the official `native` loader. Press **F10** in the main or pause menu.

Settings descriptions v1-v5 and Workspace V1 retain their data/ABI contracts. Providers that hardcode the vanilla DLL name or pin its hash need their own integration update. See [integration notes](docs/levilamina-provider-migration.md). Only one edition can run at a time; restart Minecraft to update the DLL.

Offline checks do not establish gameplay acceptance. Follow [TESTING.md](TESTING.md) for the first manual test. Unknown game images and native hook conflicts are refused. No resource pack is required.

---

基於 LeviLamina 正式原生插件方式製作的獨立 ModMenu，適用於 **Minecraft 1.26.60.05／LeviLamina 26.60.7／Windows x64**。測試版：**0.1.0-test2-ll26.60**。

可在主選單與暫停選單瀏覽已載入插件、修改支援的設定，並開啟插件宣告的 Workspace。保留遊戲語言切換、快捷鍵組合、滑動條、文字欄位，以及材質包按鈕入口 `button.modmenu.open`。

完全退出遊戲後，將套件解壓到該實例根目錄，確認為 `mods/ModMenuBedrockLL/`。啟動前停用原版 ModMenuBedrock；兩個版本不可同時使用，也不要重新命名 DLL。在主選單或暫停選單按 **F10** 開啟。

[下載本版](https://github.com/XuanDiNa/ModMenuBedrock/releases/tag/v0.1.0-test2-ll26.60) · [上次公開版之後的累積更新](docs/releases/2026-11-14-cumulative.md) · [原版路線](https://github.com/XuanDiNa/ModMenuBedrock/tree/main)

已有本獨立版本時，使用檔名含 `update` 的套件保留 `config.json`；首次安裝使用 `windows-x64` 完整套件。這次公開的是獨立 LeviLamina 版，沒有把原版 DLL 或其他插件混入套件。

設定描述檔 v1-v5 與 Workspace V1 的資料格式及 ABI 保持相容。若提供者寫死舊 DLL 名稱或雜湊，需由該插件另行適配，本套件不會修改它。原版設定與套件不會被覆寫。

自動化結果屬離線驗證，遊戲內效果仍待人工驗收。請依[測試步驟](TESTING.md)操作；未知映像與原生掛鉤衝突會安全拒絕。使用時不需要材質包。

MIT license for ModMenu sources. LeviLamina is a separately installed dependency under its own license.
