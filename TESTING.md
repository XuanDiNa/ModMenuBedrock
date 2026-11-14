# 0.1.0-test2-ll26.60 manual test / 人工測試

Windows x64, Minecraft **1.26.60.05**, official **LeviLamina 26.60.7**. Gameplay acceptance pending.

**test2 targeted check:** keep the affected player's EXE, launcher console option
and other plugins unchanged. Replace only the independent ModMenuLL package
after fully exiting Minecraft; retain the current `config.json` and logs. The
new log must say `0.1.0-test2-ll26.60` and pass `Verified native profile:
1.26.60.05`. Test Mods/F10 in the main menu and one full restart. If another
guard refuses, return that new complete log; do not change game files.

**test2 玩家驗收：** 完全退出遊戲後，只更新此獨立 ModMenuLL 套件，保留現有
`config.json` 和日誌。保持玩家原 EXE、控制台選項及其他插件不變；新日誌須顯示
`0.1.0-test2-ll26.60` 及 `Verified native profile: 1.26.60.05`。測主選單
Mods／F10，再完整重啟一次。如果出現新的拒絕原因，回傳完整新日誌。

1. Fully exit Minecraft. Preserve and disable vanilla ModMenu; keep LeviLamina enabled.
2. Extract to the instance root: `mods/ModMenuBedrockLL/`. Do not rename the DLL.
3. Main menu: check Mods, F10, clicks, scrolling and Escape/Done returning to the original menu.
4. Configure: test Cancel/Apply, slider, text editing/paste and Ctrl/Shift/Left Alt chords. Reopen to verify persistence; actual hot reload depends on the provider.
5. World pause menu: repeat F10/return. F10 must stay blocked in HUD, inventory and vanilla settings.
6. Switch to browser and return. Keep the page if its original scene is valid, but clear held-key/drag/capture transients. Changing world or scene must not restore an invalid page.
7. Workspace: use an explicitly adapted provider. Providers pinning the vanilla DLL need their own update. Do not bypass their admission checks.
8. If using a resource-pack entry, test main and pause menus. Fully exit and restart once.

1. 完全退出遊戲，保留並停用原版 ModMenu，啟用 LeviLamina 26.60.7。
2. 解壓到 1.26.60.05 實例根目錄，確認為 `mods/ModMenuBedrockLL/`，不要改 DLL 名稱。
3. 主選單測試 Mods 按鈕、F10、點擊、捲動、Escape／Done 返回。
4. Configure 測試 Cancel、Apply、滑動條、文字貼上、Ctrl／Shift／左 Alt 組合鍵，重開確認儲存。
5. 進世界後在暫停選單再測一次；HUD、背包、原版設定頁仍不得開啟 Mods。
6. 切到瀏覽器再回來，確認有效頁面保留，按下／拖動／綁鍵暫態清除。更換世界或場景後，不應恢復失效頁面。
7. Workspace 只測已明確適配此 DLL 的提供者；固定舊 DLL 雜湊的插件需另行更新。
8. 如有材質包入口，測試主選單與暫停選單，最後完整重啟一次。

異常時提供 `mods/ModMenuBedrockLL/ModMenuBedrockLL.log` 與 LeviLamina 載入日誌，並註明控制台選項是否啟用。保留原設定即可。

For failures, provide the edition's log and LL loader log, noting the console option. Preserve your settings.
