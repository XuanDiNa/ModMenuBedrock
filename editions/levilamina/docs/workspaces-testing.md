# 0.6.0-test3 manual acceptance / 人工驗收

## Focus loss and return / 失焦與恢復

From the main menu, open a provider Workspace, select a non-default page and
scroll down. Switch to a browser for at least five seconds, then return. The
same page and scroll must remain, with updated provider state. Repeat from the
pause menu. Background login/job completion must be checked with the real provider;
offline UI tests do not prove Microsoft authorization succeeds.

Repeat while holding a mouse button, Enter or F10: returning/releasing must not
submit an action or toggle a page; a fresh press after release must work. Cancel
key capture, an unfinished text edit, slider drag and dangerous confirmation on
focus loss; preserve previously committed local form values/Configure drafts.
No automatic Apply, Stop, retry or command replay is permitted.

Close Mods and change scene/world, open vanilla settings/inventory or resume the
world before returning to an allowed menu: no old Workspace may auto-restore.
Provider unregister/re-register must not inherit its old session page. Do not
force DLL unload; accepted providers are pinned by the existing API.

在主選單開啟工作頁，切到其他分頁並向下捲動，再切換瀏覽器至少五秒，回來後
應保留原分頁／捲動並顯示最新狀態；暫停選單也測一次。真實登入或任務結果需由
提供者實測，離線 UI 測試不代表 Microsoft 授權已成功。

按住滑鼠、Enter 或 F10 切換：回來及放開不得執行動作，放開後的新操作應正常。
失焦須取消綁鍵、未完成文字編輯、滑動條拖動與危險確認；保留已確認表單值及
Configure 草稿，不自動 Apply／Stop／重試或重送命令。

關閉 Mods 並切換世界／場景、進入原版設定／背包或恢復遊戲，再回到允許選單時，
不得自動恢復舊工作頁。提供者重新註冊不得沿用舊 session；不要強制卸載 DLL。

BedrockToolNative 0.2.0-test3 pins ModMenu test2. This new test3 DLL requires a
separate BedrockTool dependency review and updated exact hash before joint testing.
Do not bypass its hash guard. The optional example can test focus behavior alone.

BedrockToolNative 0.2.0-test3 固定 ModMenu test2；聯合測試前，BedrockTool 需另行
審核並固定這個新 DLL 的精確雜湊，不可繞過保護。可先以選用示範獨立驗收失焦行為。

## Focused pointer regression / 本次座標修正

Move the pointer after opening Workspace, then click an action below read-only
status rows, the page arrows and Back. The action must run only on a paired
press/release; Back returns to Mods. Moving alone must never submit a command.
Repeat from the pause menu; keyboard activation must still work. Keep the
existing busy, pending, revision and dangerous-action confirmation checks.

開啟工作頁後移動滑鼠，點擊唯讀狀態列下方的操作、左右翻頁及 Back；操作只在
成對按下／放開後執行，Back 回到 Mods，單純移動不得提交命令。暫停選單再測一次，
鍵盤操作仍須可用；busy、pending、revision 與危險操作確認維持原有規則。

Older BedrockTool builds pin a previous ModMenu binary. Joint testing requires a
separately reviewed BedrockTool build that pins this new DLL's SHA-256; updating
ModMenu alone is not a compatible pair. Do not weaken the hash guard.

舊 BedrockTool 固定舊 ModMenu 雜湊，聯合測試需先由該專案審核並固定新版 DLL
的 SHA-256；只更新 ModMenu 不是可驗收的配套，不可放寬雜湊保護。

## English

This is a new candidate, not the accepted 0.5.0 DLL. No installation is automatic.
Back up the current ModMenu folder before replacing only its DLL/metadata.
Preserve `config.json` and logs. Close Minecraft normally before copying.
Keep only the manifest name matching the launcher state (`manifest.json` or
`manifest.json.close`), never both. The package intentionally has no root config.

The separate optional `ModMenuWorkspaceExample` folder contains a real DLL, not
a fake catalog entry. Install it only to test the generic interface. It performs
no network/login/export/script operations. Its Configure text setting is a save
example only; the Workspace job-name field is an independent draft.

For each exact supported build (1.26.45.01 / 1.26.51.01 / 1.26.52.03):

1. Launch twice. Check Mods button/F10 at main and pause menus, and no activation
   from HUD, inventory or vanilla settings. Check existing plugins still work.
2. Select ModMenu Workspace Example. Configure: edit the text, test Unicode paste,
   Home/End, arrows, Shift selection and Ctrl+A/C/V. Cancel must not save; Apply
   must save and preserve unknown config data. Test the byte-limit error.
3. Workspace: merely open/refresh/switch pages; status remains Idle. Edit a job
   name and click Start; progress runs for about five seconds. Close the page
   during progress and reopen; the job continues. Stop works only explicitly.
4. Results/image page: select a completed row, scroll, inspect the bounded raster.
   Return to Jobs, activate Reset results: Cancel/Escape must not clear; Confirm
   must clear. Do not confuse an action result with real network-job testing.
5. Resize and change UI scale, especially at compact viewports. Check no stuck
   click, leaked keyboard/mouse input or extra action after page/focus changes.
6. Remove only the optional example after normal shutdown. Ordinary v1/v2 mods
   must remain usable without updating their DLL or descriptors.

Current BedrockTool native builds can reject the new ModMenu hash. Keep them
disabled for this standalone interface acceptance unless a separately reviewed
adapter is supplied. Do not change its guard to force compatibility.

Rollback: after normal game shutdown restore the previous ModMenu DLL/metadata;
remove the optional example folder only. Retain current configs/logs/backups.

## 繁體中文

這是新測試版，不是已驗收的 0.5.0 DLL；不會自動安裝。先正常退出遊戲並備份現有
ModMenu，再只換 DLL／中繼檔，保留 `config.json` 與日誌。
依啟動器狀態只保留 `manifest.json` 或 `manifest.json.close`，不要兩份同時存在。
主套件刻意不附根目錄設定檔，以免覆蓋既有設定。

選用的 `ModMenuWorkspaceExample` 是真正可載入的示範 DLL，不連網、不登入、
不匯出、不執行腳本。Configure 的文字儲存示範與工作頁名稱草稿彼此獨立。

三個精確版本各測一次：

1. 啟動兩次；主選單／暫停選單的 Mods／F10 正常，HUD／背包／原版設定不開啟；
   原有插件維持正常。
2. 示範插件 Configure：測 Unicode 貼上、方向鍵、Home／End、Shift 選取、
   Ctrl+A／C／V。Cancel 不儲存、Apply 儲存，原有未知設定不丟失；超長輸入顯示錯誤。
3. 工作頁：單純開頁／切頁不開始工作。修改名稱並按 Start，約五秒完成；中途關頁
   再開，工作應持續。只有明確按 Stop 才停止。
4. 結果／圖像頁：捲動、選取完成結果並查看有界像素預覽。回工作頁按清除結果，
   Cancel／Escape 不清除，Confirm 才清除。
5. 調整視窗與 UI 比例；檢查無卡住、誤操作、輸入穿透或關頁後多執行一次。
6. 正常退出後移除選用示範，舊 v1／v2 插件無須更新仍能使用。

現有 BedrockTool 原生版可能拒絕新 ModMenu 雜湊；本輪獨立驗收請暫停載入它，
除非已有另外審核的適配版。不要放寬它的檢查來強制使用。

回復方式：退出遊戲後還原舊 ModMenu DLL／中繼檔，只移除選用示範；保留設定與日誌。
