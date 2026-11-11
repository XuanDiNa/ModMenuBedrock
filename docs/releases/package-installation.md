# ModMenuBedrock 0.6.0-test5-leftalt

[Release and source](https://github.com/XuanDiNa/ModMenuBedrock/releases/tag/v0.6.0-test5-leftalt)

## English

Windows x64 / LeviLauncher native plugin update. Prerelease; in-game acceptance
of these new DLLs is pending. No automatic installation or game launch.

The menu-only ZIP contains `ModMenuBedrock/` and retains the exact supported
Minecraft builds 1.26.45.01 / 1.26.51.01 / 1.26.52.03. The matching bundle
contains `mods/` and targets an existing **1.26.52.03** installation. It updates
ModMenu and seven optional consumers: AutoClick, AutoJumpReset, ContainerPreview,
FakeSneak, FullBright, ToggleSprint and MuoniumPlayerBedrock.

1. Exit Minecraft. Back up affected plugin folders outside the active `mods`.
2. Merge the package's plugin folders into your version's existing `mods` folder.
   Replace each selected DLL, manifest and settings description together. Keep
   all original settings, logs, profiles, runtime and other player files.
3. Preserve enabled/disabled state: if the launcher uses `manifest.json.close`,
   update that manifest without leaving an additional enabled `manifest.json`.
   Keep only one active copy of each DLL. Do not replace whole plugin folders.
4. For a complete matching-bundle installation, run the read-only checker:

   ```powershell
   powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\CHECK-INSTALL.ps1 -ModsPath 'your-version-directory\mods'
   ```

   It checks all eight sets. Intentionally omitted or disabled plugins may be
   reported as missing; do not enable them solely to satisfy the checker.
5. Restart via LeviLauncher. Click Mods in the main/pause menu or press F10.
   In Configure, hold Ctrl/Shift/Left Alt before pressing a main key. Apply saves,
   Cancel discards, Esc during capture selects Unbound. Test one plugin at a
   time, including actual action, Apply/reopen and clearing the binding.

**Muonium is a native DLL update for an existing 0.7.0-test1 installation, not
a complete player.** This package contains no Java program, runtime, music,
fonts or other player assets, and does not grant rights to separate components.
Keep their original licenses and files. Only the native binding update is supplied.
The retained player NOTICE describes the separate full player; entries for
Java/assets/runtime do not mean those files are contained in this archive.

ContainerPreview still uses hold-to-preview and its original sneak/Shift guard
for new background opens. Test that path with Ctrl+K or LeftAlt+K.
Windows-reserved shortcuts may be consumed by the OS; Right Alt/AltGr does not
substitute for Left Alt. No new game-version support is added.

**BedrockToolNative 0.2.0-test5 only accepts the older ModMenu test3 hash and
rejects this DLL.** Its companion admission is not updated. Do not bypass it.
For rollback, exit the game and restore DLL, manifest, description and settings
together. Older DLLs may not understand newly saved key combinations.

All eight native DLLs retain the frozen test candidates' exact hashes. Public
packaging only adds dependency licenses and installation documentation. No
user settings or account data are included. SHA256SUMS.txt identifies payload
files; DLL-MANIFEST.json identifies the eight matching sets where included.

## 繁體中文

Windows x64／LeviLauncher 原生插件更新。預發佈版，新 DLL 尚待遊戲內人工驗收；
不會自動安裝或啟動遊戲。

單獨包含 `ModMenuBedrock/`，維持支援 1.26.45.01／1.26.51.01／1.26.52.03；
配套包含 `mods/`，只用於既有 **1.26.52.03**，包含 ModMenu 及上列七個選用插件。

1. 完全退出遊戲，把原有插件備份到有效 mods 以外。
2. 合併對應插件目錄，一起替換 DLL、manifest 與描述檔；保留設定、日誌、profile、
   runtime 及其他播放器資料，不要整個資料夾刪掉重放。
3. 保留啟動器啟用／停用狀態。若原本是 `manifest.json.close`，更新該份 manifest，
   不要額外留下啟用的 `manifest.json`。有效目錄內只保留一份插件 DLL。
4. 完整安裝配套包時可執行上方唯讀核對指令。自行略過或停用的插件可能顯示缺少，
   不必為了通過核對而啟用它們。
5. 透過 LeviLauncher 重啟。主選單／暫停選單點 Mods 或按 F10；設定組合鍵時，
   先按住 Ctrl／Shift／左 Alt 再按主鍵。Apply 儲存、Cancel 捨棄，綁定時 Esc
   設為無綁定。逐一測實際功能、套用後重開保留及解除綁定。

**Muonium 是既有 0.7.0-test1 的原生 DLL 更新，不是完整播放器。** 不含 Java、
runtime、音樂、字型或其他播放器資源，也不授予那些獨立組件的再散布權利。
請保留原有檔案及授權；隨附原 NOTICE 對完整播放器的說明不代表包內包含那些檔案。

容器預覽維持按住生效，背景開箱的潛行／Shift 保護不變，該路徑請用 Ctrl＋K
或左 Alt＋K 測試。系統可能攔截 Windows 保留鍵；右 Alt／AltGr 不代替左 Alt。
此更新不新增遊戲版本支援。

**BedrockToolNative 0.2.0-test5 綁定舊 ModMenu test3 雜湊，會拒絕本 DLL。**
本包未改 companion 檢查，請勿繞過。回退時退出遊戲，一起還原 DLL、manifest、
描述檔及設定；舊 DLL 可能無法解析新版儲存的組合鍵。

八個 DLL 保持凍結測試候選版的相同雜湊，公開包只補授權及安裝文件，不含使用者
設定或帳戶資料。SHA256SUMS.txt 提供校驗碼，配套包另有 DLL-MANIFEST.json。
