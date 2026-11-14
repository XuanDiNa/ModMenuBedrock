# Resource-pack entry test / 材質包入口測試

Candidate: **0.6.0-test7-resourcepack**. Windows x64, exact Minecraft builds
**1.26.45.01 / 1.26.51.01 / 1.26.52.03**. Not yet accepted in gameplay.

## English

1. Exit Minecraft normally. Back up the current ModMenu folder. Update only
   `ModMenuBedrock.dll`, `manifest.json` and `mod.json` from the update ZIP;
   preserve `config.json`, logs and launcher enabled/disabled state. Keep one
   active DLL. The ZIP contains no other plugin or default configuration.
2. Import `ModMenuEntryExample-1.0.0.mcpack` and enable it under **Global
   Resources**. Restart Minecraft. Begin with the vanilla layout; a custom
   menu replacement needs to integrate the provided button into its own layout.
3. In the main menu, click the new top-left Mods button. Check that it opens
   Mods and Done returns to the same main menu. Check the configured shortcut
   as well (F10 by default).
4. In a world, open the pause menu and click its Mods button. Escape must return
   to the pause menu with the game still paused. Test keyboard/controller
   selection if used, plus your usual GUI scale, window size and game language.
5. Check there is only one pack entry per menu and no added entry in vanilla
   settings, inventory or HUD. Existing Configure and Workspace pages should
   retain their normal behavior. Providers that pin a particular ModMenu DLL
   hash need their own matching update; do not bypass their checks.
6. Disable the example and restart. On a supported vanilla layout, the normal
   automatic Mods entry and shortcut should work again. Then test the final
   custom pack stack after its author has integrated the entry.

Report the exact game build, pack stack and main/pause click results separately,
with the candidate log header. If a button is absent, note whether the shortcut
works. This interface does not automatically insert entries into arbitrary packs.
Engine merging, appearance and actual native event delivery require this manual
test; offline verification is not gameplay acceptance. No GitHub upload or
automatic installation is part of this candidate.

## 繁體中文

1. 正常退出 Minecraft，備份目前 ModMenu。從更新包只替換
   `ModMenuBedrock.dll`、`manifest.json`、`mod.json`；保留 `config.json`、日誌及
   啟動器啟用／停用狀態，只保留一份載入中的 DLL。包內沒有其他插件或預設設定檔。
2. 匯入 `ModMenuEntryExample-1.0.0.mcpack`，在**全域資源**啟用，然後重新啟動遊戲。
   先使用原版排版；若材質包完全替換選單，需要由作者把範例入口整合進自己的排版。
3. 主選單點擊左上角 Mods，確認能進入 Mods，Done 返回同一主選單；再測原有快捷鍵
   （預設 F10）。
4. 進入世界並開啟暫停選單，點擊 Mods；Escape 應回到暫停選單，遊戲仍暫停。
   有使用鍵盤／控制器選取時也請測試，並檢查平常的 GUI 比例、視窗大小及遊戲語言。
5. 每個選單只應有一個材質包入口，原版設定／背包／HUD 不應新增入口。原有 Configure
   及 Workspace 應正常。若提供者限定 ModMenu DLL 雜湊，需要另行取得相容版本，
   請保留它的檢查。
6. 停用範例並重啟；受支援的原版排版應恢復自動 Mods 入口及快捷鍵。
   自訂材質包作者完成接入後，再測實際材質包組合。

回報精確遊戲版本、材質包組合、主選單／暫停選單各自的點擊結果，並確認日誌為本次
候選版本。若沒有按鈕，請同時註明快捷鍵能否使用。這個介面不會自動替所有材質包
插入入口。離線驗證不代表遊戲內合併、外觀及點擊已通過人工驗收；本次沒有自動安裝
或上傳 GitHub。
