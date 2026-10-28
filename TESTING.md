# Testing / 測試

## English

On 2026-10-29 the maintainer reported normal basic use of **this exact
0.5.0-test1 DLL** on Minecraft 1.26.45.01, 1.26.51.01 and 1.26.52.03.
This is a general three-version result, not a recorded per-step performance
benchmark or exhaustive acceptance of every environment.

For regression checks:

1. Close Minecraft before installation. Back up only ModMenu and preserve its
   configuration, logs and launcher enabled/disabled state.
2. Check main/pause Mods buttons and F10 (or your custom shortcut), including a
   cold restart. Escape from Mods should return to the original pause menu.
3. Verify loaded-only discovery, search and the current game-language labels.
4. In Configure, check Cancel discards drafts and Apply persists values. For key
   fields that allow no binding, Escape during capture should clear the key.
5. For an explicitly integrated slider, check slow/fast drag, release outside,
   Escape during drag, direct input, keyboard steps and Apply/Cancel. The optional
   [AutoRestock descriptor](integration/v2-examples/AutoRestock/modmenu.settings.json)
   matches manifest `0.3.1-test4` only; back up its descriptor and leave its DLL
   and config unchanged. Do not fake a manifest version to force acceptance.
6. Close Mods and check the installed compatible companion plugins still work.
   A successful JSON save does not prove a companion hot-reloaded it.

Long sessions, all GUI scales, all companion combinations, fullscreen modes,
OreUI and arbitrary custom menu resource packs are not exhaustively verified.
No server-protocol or multiplayer-wide behavior is claimed by this UI test.
Do not bypass an INACTIVE safety refusal; report the version and log reason.

Automated commands and the distinction between synthetic/private-image tests
are in [BUILDING.md](BUILDING.md). No automated test is a replacement for playtesting.

## 繁體中文

2026-10-29 維護者回報：**此份 0.5.0-test1 DLL** 在 1.26.45.01、1.26.51.01、
1.26.52.03 三個版本基本使用正常。這是整體回報，不自行推定每一步都分別記錄過，
也不是效能數據或全部環境的完整驗收。

回歸測試可依序確認：

1. 關閉遊戲後安裝，備份 ModMenu，保留設定、日誌與啟用／停用狀態。
2. 主選單／暫停選單 Mods、F10 或自訂鍵，並完全退出後重啟一次；Escape 返回原本暫停選單。
3. 只列已載入插件、搜尋及跟隨遊戲語言的文字。
4. Configure 的 Cancel 不儲存、Apply 保存；允許無綁定的欄位在等待綁鍵時按 Escape 清空。
5. 已明確接入的滑動條測慢拖／快拖、滑出放開、拖動中 Escape、數字輸入、鍵盤與
   Apply／Cancel。選用 AutoRestock 描述檔僅符合 manifest `0.3.1-test4`，先備份描述檔，
   不換 DLL 或 config，也不要偽造版本。
6. 關閉 Mods 後檢查原有相容插件；JSON 儲存成功不代表插件確實熱載入。

長時間使用、所有 GUI 比例／插件組合／全螢幕模式、OreUI 及任意改版選單仍未全面驗證。
此介面測試不宣稱伺服器協定或所有多人環境均通過。遇到 INACTIVE 應回報日誌原因，
不要取消保護強行啟用。離線驗證方式見 [BUILDING.md](BUILDING.md)，不能代替人工遊玩。
