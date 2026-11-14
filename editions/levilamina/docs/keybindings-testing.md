# Keyboard binding candidate / 快捷鍵候選版

## English

This is a new test candidate, not a gameplay-accepted release. Install manually
after closing Minecraft. Keep a copy of the current DLLs, manifests, sidecars and
JSON settings. The package contains no configuration files to overwrite yours.

For the current flat 26.52 matching bundle, follow the README at the archive
root and run its read-only `CHECK-INSTALL.ps1`. That bundle rebases Muonium onto
the installed 0.7 release. Earlier frozen nested bundles retain their original
0.6 player candidate and must not replace a newer installation.

1. Copy the `ModMenuBedrock` update into the existing ModMenu loader folder.
   Remove the older copy only from the active loader location; do not load two
   ModMenu versions. Keep the backup elsewhere for rollback.
2. Optional plugin updates are separate. For each plugin you test, replace its
   DLL, exact-version manifest and matching `modmenu.settings.json` together.
   Keep existing JSON and other files. Do not apply a v5 sidecar to an older DLL.
3. Restart the game for DLL changes. JSON edits through Apply retain the
   plugin's existing hot-reload timing. Cancel does not save.

| Candidate | Existing game boundary |
| --- | --- |
| ModMenuBedrock | 1.26.45.01 / 1.26.51.01 / 1.26.52.03 |
| ContainerPreview | Same three exact versions |
| AutoClick | Retains its existing exact native screen/slot guards |
| AutoJumpReset v2, FakeSneak, FullBright, ToggleSprint | 1.26.52.03 |
| MuoniumPlayerBedrock | 1.26.52.03; current flat bundle uses frozen 0.7.0-test1 sources |

The current Muonium update is 0.7.0-test2-leftalt for an existing 0.7.0-test1
installation; preserve its runtime and Java files. Its native, Java, world and
environment source files are unchanged. Earlier frozen packages used 0.6.0-test4
as their base; check the archive README before applying an update.

The installed BedrockToolNative test5 has an exact ModMenu test3 hash/size gate.
It will reject this new ModMenu hash. Do not weaken that check. Joint Workspace
testing needs a separately reviewed companion update; this package does not
update BedrockTool admission.

### Manual checks

- In a binding-enabled Configure field, bind `Ctrl+K`, `Shift+K` and
  `Ctrl+Shift+K`, then `LAlt+K`, `Ctrl+LAlt+K`, `Shift+LAlt+K` and
  `Ctrl+Shift+LAlt+K`; Apply, reopen, and check the caption. Cancel a change once.
- In the plugin's allowed gameplay context, the complete combination should
  act once. Bare K, extra Ctrl/Shift/Left Alt, holding/repeating, or pressing K before
  the modifiers should not trigger that binding. Release then press again.
- Bind Backspace, a navigation key or punctuation. Bind Ctrl/Shift/Left Alt alone by
  pressing and releasing it. Esc during capture selects Unbound and stays on
  Configure; Apply persists it. Mouse buttons cannot be captured.
- Switch to another app with a key held, return, and check that nothing acts
  until a fresh release/press. Confirm original menu, pause, settings and input
  gates remain normal. Try the menu's own open shortcut separately.
- ContainerPreview is a hold feature: release any required key to stop.
  Its existing native sneak/Shift safety guard still pauses new background
  container opening; use `Ctrl+K` when testing that path. This binding update
  does not remove the container-interaction guard.
- Right Alt/AltGr plus K must not activate a Left Alt chord.
- Test the existing plugin action once after editing its binding. This does
  not require repeating unrelated completed gameplay campaigns.

Windows may consume system shortcuts before the game observes them. Firmware
Fn is not a normal Windows keyboard key. Neither can be guaranteed here.

For rollback, exit the game and restore the previous DLL, manifest and sidecar
as a set. Restore your backed-up JSON if it now contains a chord that the old
runtime cannot read. No files in the game installation are changed automatically.

## 繁體中文

這是新的人工測試候選版，尚未經遊戲驗收。先退出遊戲，備份原有 DLL、manifest、
描述檔與 JSON。主包不包含要覆蓋你的設定檔。

目前的 26.52 平面配套包請依壓縮包根目錄 README 安裝，並執行唯讀的
`CHECK-INSTALL.ps1`。此包的 Muonium 基於已安裝的 0.7 版本；先前凍結的巢狀包
仍保留 0.6 候選版，不要拿它覆蓋較新的播放器。

先更新 ModMenu。要測試的其他插件，請把對應 DLL、manifest 與 v5 描述檔一起更新，
保留原有 JSON 與其他執行檔案；不可只把新版描述檔配上舊 DLL。更換 DLL 後重啟遊戲。

依序測試 Ctrl＋K、Shift＋K、Ctrl＋Shift＋K，再測左 Alt＋K、Ctrl＋左 Alt＋K、
Shift＋左 Alt＋K、Ctrl＋Shift＋左 Alt＋K 的套用、重開保留與取消。完整組合只觸發一次；
單按 K、多按另一個 Ctrl／Shift／左 Alt、持續按住或先按 K 再補修飾鍵均不觸發該組合。
Backspace、導覽及標點鍵可綁；單獨 Ctrl／Shift／左 Alt 在放開時確認。Esc 選無綁定並停留設定頁，
按套用才儲存。失焦返回後須放開再按，滑鼠鍵不可綁。

右 Alt／AltGr＋K 不應觸發左 Alt組合。

容器預覽維持按住生效，任一必要鍵放開即停止。原有 Shift／潛行背景開箱保護仍保留，
請用左 Alt＋K 或 Ctrl＋K 測背景開箱。新版平面包的 Muonium 只更新 0.7.0-test1
的快捷鍵，保留原有 runtime 與 Java 檔案，沒有併入正在開發的其他功能。

BedrockToolNative test5 只接受固定的 ModMenu test3 雜湊，會拒絕這次新 DLL；
此包沒有修改該檢查。請先分開驗收快捷鍵，聯合 Workspace 測試需另外更新 companion。

回退時退出遊戲，一併還原 DLL、manifest、描述檔；若已儲存組合鍵，也還原舊 JSON 備份。
離線建置、解析與原生映像檢查不代表遊戲測試通過。
